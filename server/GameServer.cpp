#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "GameServer.h"
#include "GameBoard.h"

using namespace std;

GameServer::GameServer()
{
  commandHandler = CommandHandler::getInstance();
  gameBoardFactory = GameBoardFactory::getInstance();
}

void GameServer::clientThreadMainLoop(int sock)
{
  PlayerData* playerData = NULL;   // Data for this client.
  char buffer[1024];
  char* dataAfterNewLine = NULL;
  while (1)
  {
    // Wait for socket input.
    if (dataAfterNewLine == NULL)
    {
      memset(buffer, 0, sizeof(buffer));
      int n = read(sock, buffer, sizeof(buffer) - 1);
      if (n < 0)
      {
        perror("ERROR reading from socket");
        exit(1);
      }
    }
    else
      strcpy(buffer, dataAfterNewLine);

    // Parse receved command from client.
    printf("Got command from client: %s\n", buffer);
    char* restOfLine;
    Command command =
      commandHandler->parseCommand(buffer, (playerData != NULL),
                                   restOfLine, dataAfterNewLine);
    commandMutex.lock();
    switch (command)
    {
      case NoDataCommand:
        break;
      case AmbigousCommand:
        sendMessageToClient(sock, "server: Ambigous command '%s'", buffer);
        break;
      case NameNotGivenYetCommand:
        sendMessageToClient(sock, "server: Tell me your name first");
        break;

      // Informational commands
      case HelpCommand:       showCommands(sock); break;
      case WhoCommand:        showPlayers(sock); break;
      case GamesCommand:      showGames(sock); break;
      case ChallengesCommand: showChallenges(playerData); break;

      // Player related commands
      case NameCommand: addPlayer(restOfLine, sock, playerData); break;
      case GameCommand: setFavouriteGame(playerData, restOfLine); break;

      // Game playing related commands
      case ChallengeCommand: challengeOtherPlayer(playerData, restOfLine); break;
      case RecallCommand: recallChallenge(playerData, restOfLine); break;
      case AcceptCommand: acceptChallenge(playerData, restOfLine); break;
      case RejectCommand: rejectChallenge(playerData, restOfLine); break;
      case PlayCommand:   makePlayerMove(playerData, restOfLine); break;
      case BoardCommand:  showBoard(playerData); break;
      case ResignCommand: resignGame(playerData); break;

      // Other commands
      case TellCommand:
      {
        char* otherPlayerName;
        char* message;
        commandHandler->getFirstWord(restOfLine, otherPlayerName, message);
        tellPlayer(playerData, otherPlayerName, message);
        break;
      }

      default:
        sendMessageToClient(sock, "server: Unknown command '%s'", buffer);
        break;
    }
    commandMutex.unlock();
  }
}

void GameServer::showCommands(int sock)
{
  sendMessageToClient(sock, "%s", commandHandler->getCommandHelp());
}

void GameServer::showPlayers(int sock)
{
  char buffer[8192];
  int len = sprintf(buffer, "The players are:\n");
  for (auto iterator=players.begin(); iterator!=players.end(); ++iterator)
  {
    const char* playerName = iterator->first;
    len += sprintf(&buffer[len], "%-16s: ", playerName);

    // Check what game player is playing or wants to play.
    PlayerData* playerData = iterator->second;
    char* gameName = playerData->getFavouriteGame();
    GameData* gameData = playerData->getOngoingGame();
    if (gameName == NULL)
      len += sprintf(&buffer[len], "Game not selected\n");
    else
    {
      char playingData[1024] = {0};
      if (gameData != NULL)
        sprintf(playingData, " (playing with %s)",
                gameData->getOtherPlayer(playerName));
      len += sprintf(&buffer[len], "%s%s\n", gameName, playingData);
    }
  }
  sendMessageToClient(sock, "%s", buffer);
}

void GameServer::showGames(int sock)
{
  // Ask GameBordFactory for all known games.
  sendMessageToClient(sock, "You can play these games:\n%s",
                      gameBoardFactory->getGameList());
}

void GameServer::showChallenges(PlayerData* myData)
{
  bool challengeFound = false;

  // Show if player has challenged anyone.
  auto iterator = challenges.find(myData->getPlayerName());
  if (iterator != challenges.end())
  {
    challengeFound = true;
    sendMessageToClient(myData->getSocket(), "You have challenged %s\n",
                        iterator->second->getChallengee());
  }

  // Show if player has been challenged by anyone.
  const char* name = myData->getPlayerName();
  for (iterator=challenges.begin(); iterator!=challenges.end(); ++iterator)
    if (strcmp(name, iterator->second->getChallengee()) == 0)
    {
      challengeFound = true;
      sendMessageToClient(myData->getSocket(),
                          "You have been challenged by %s\n",
                          iterator->second->getChallenger());
    }

  if (!challengeFound)
    sendMessageToClient(myData->getSocket(), "No challenges found");
}

void GameServer::addPlayer(char* playerName, int sock, PlayerData*& playerData)
{
  printf("Adding player %s\n", playerName);

  // Name changes aren't allowed.
  if (playerData != NULL)
  {
    sendMessageToClient(sock, "server: You can't change your name %s",
                        playerData->getPlayerName());
    return;
  }

  // Spaces in name isn't allowed.
  char* firstWord;
  char* restOfString;
  commandHandler->getFirstWord(playerName, firstWord, restOfString);
  if (*restOfString != '\0')
  {
    sendMessageToClient(sock, "server: Spaces are not allowed in player name");
    return;
  }

  if (playerExist(playerName))
  {
    sendMessageToClient(sock, "server: Player %s already exists", playerName);
    return;
  }

  // Create new PlayerData and store name and socket.
  playerData = new PlayerData(playerName, sock);
  players[strdup(playerName)] = playerData;
}

void GameServer::setFavouriteGame(PlayerData* myData, char* nameOfGame)
{
  // Validate that we know the rules of the selected game.
  const char* game = gameBoardFactory->getFullNameOfGame(nameOfGame);
  if (game != NULL)
  {
    myData->setFavouriteGame(game);
    sendMessageToClient(myData->getSocket(),
                        "server: Setting favourite game to %s", game);
  }
  else
    sendMessageToClient(myData->getSocket(),
                        "server: I don't know the rules of %s", nameOfGame);
}

void GameServer::challengeOtherPlayer(PlayerData* myData, char* challengee)
{
  // Get data for other player.
  PlayerData* opponentData = getPlayerData(challengee);
  if (opponentData == NULL)
  {
    sendMessageToClient(myData->getSocket(),
                        "server: I don't know who %s is", challengee);
    return;
  }

  // Player must be another client.
  if (myData == opponentData)
  {
    sendMessageToClient(myData->getSocket(),
                        "server: You're not allowed to challenge yourself");
    return;
  }

  // Ensure a favourite game has been selected.
  if (myData->getFavouriteGame() == NULL)
  {
    sendMessageToClient(myData->getSocket(),
                        "server: Tell me your favourite game first");
    return;
  }

  // Ensure other player wants to play the same game.
  if (strcmp(myData->getFavouriteGame(),
             opponentData->getFavouriteGame()) != 0)
  {
    sendMessageToClient(myData->getSocket(), "server: %s wants to play %s",
                        challengee, opponentData->getFavouriteGame());
    return;
  }

  // Check that we're not already playing a game.
  char* challenger = myData->getPlayerName();
  GameData* gameData = myData->getOngoingGame();
  if (gameData != NULL)
  {
    sendMessageToClient(myData->getSocket(),
                        "server: You're already playing a game against %s",
                        gameData->getOtherPlayer(challenger));
    return;
  }

  // Check that challengee isn't already playing a game.
  gameData = opponentData->getOngoingGame();
  if (gameData != NULL)
  {
    sendMessageToClient(myData->getSocket(),
                        "server: %s is already playing a game against %s",
                        challengee, gameData->getOtherPlayer(challengee));
    return;
  }

  // Check that we haven't sent another challenge.
  auto iterator = challenges.find(challenger);
  if (iterator != challenges.end())
  {
    sendMessageToClient(myData->getSocket(),
                        "server: You have already challenged %s",
                        iterator->second->getOtherPlayer(challenger));
    return;
  }

  // Check that challengee hasn't challenged us already.
  iterator = challenges.find(challengee);
  if (iterator != challenges.end() &&
      strcmp(iterator->second->getChallengee(), challenger) == 0)
  {
    sendMessageToClient(myData->getSocket(),
                        "server: You have already been challenged by %s",
                        iterator->second->getChallenger());
    return;
  }

  // Create and store new challenge.
  challenges[challenger] = new GameData(myData->getFavouriteGame(),
                                        myData->getPlayerName(),
                                        opponentData->getPlayerName());

  // Send challenge to other player.
  sendMessageToClient(opponentData->getSocket(),
                      "server: %s is challenging you to a game of %s",
                      myData->getPlayerName(),
                      myData->getFavouriteGame());
  sendMessageToClient(myData->getSocket(), "server: Challenge sent to %s",
                      opponentData->getPlayerName());
}

void GameServer::recallChallenge(PlayerData* myData, char* challengee)
{
  // Check that we have sent challenge to challengee.
  char* challenger = myData->getPlayerName();
  auto iterator = challenges.find(challenger);
  if (iterator != challenges.end() &&
      strcmp(iterator->second->getChallengee(), challengee) == 0)
  {
    PlayerData* opponentData = getPlayerData(challengee);
    sendMessageToClient(opponentData->getSocket(),
                        "server: %s is recalling the challenge",
                        myData->getPlayerName());

    delete iterator->second;
    challenges.erase(challenger);
    sendMessageToClient(myData->getSocket(), "server: Challenge recalled");
  }
  else
    sendMessageToClient(myData->getSocket(),
                        "server: You haven't challenged %s", challengee);
}

void GameServer::acceptChallenge(PlayerData* myData, char* challenger)
{
  // Check if there's a game ongoing already.
  if (myData->isPlayingAGame())
  {
    sendMessageToClient(myData->getSocket(),
                        "server: You're already playing a game");
    return;
  }

  // Get all challenge data.
  PlayerData* opponentData;
  GameData* gameData;
  if (getChallengeData(myData->getSocket(), challenger,
                       opponentData, gameData))
  {
    // Do automatic recall if we've sent a challenge to someone else.
    auto iterator = challenges.find(myData->getPlayerName());
    if (iterator != challenges.end())
    {
      sendMessageToClient(myData->getSocket(),
                          "server: Recalling your challenge to %s\n",
                          iterator->second->getChallengee());
      recallChallenge(myData, iterator->second->getChallengee());
    }

    // Remove the challenge from challenges map.
    challenges.erase(challenger);

    // Send challenge accepted to other player.
    sendMessageToClient(opponentData->getSocket(),
                        "server: %s accepted the challenge to play a game of %s\n",
                        myData->getPlayerName(),
                        opponentData->getFavouriteGame());

    // Start the game and tell the players who should start playing.
    myData->setOngoingGame(gameData);
    opponentData->setOngoingGame(gameData);

    gameData->startGame();
    sendGameDataToPlayers(gameData, true, NULL);
  }
}

void GameServer::rejectChallenge(PlayerData* myData, char* challenger)
{
  PlayerData* opponentData;
  GameData* gameData;
  if (getChallengeData(myData->getSocket(), challenger,
                       opponentData, gameData))
  {
    // Send challenge rejected to opponent.
    sendMessageToClient(opponentData->getSocket(),
                        "server: %s rejected your challenge of a game of %s",
                        myData->getPlayerName(),
                        opponentData->getFavouriteGame());

    // Remove all challenge data.
    delete gameData;
    challenges.erase(challenger);
  }
}

void GameServer::makePlayerMove(PlayerData* myData, char* position)
{
  GameData* gameData;
  if (!getGameData(myData, gameData))
    return;

  // Ensure that it's my move.
  const char* myName = myData->getPlayerName();
  if (strcmp(gameData->getPlayerToMove(), myName) != 0)
  {
    sendMessageToClient(myData->getSocket(), "server: It's %s to play",
                        gameData->getPlayerToMove());
    return;
  }

  GameBoard* gameBoard = gameData->getBoard();
  if (gameBoard->makeMove(position))
  {
    // Switch player to move and send board to both players.
    bool gameOver = gameBoard->isGameOver();
    if (!gameOver)
      gameData->switchPlayerToMove();
    sendGameDataToPlayers(gameData, !gameOver, position);

    if (gameOver)
    {
      // Send game result to both players.
      int result = gameBoard->getPlayer1Result();
      PlayerData* opponentData =
        getPlayerData(gameData->getOtherPlayer(myName));

      // Tell players who won.
      bool iAmPlayer1 = (strcmp(myName, gameData->getPlayer1()) == 0);
      int mySocket = myData->getSocket();
      int opponentSocket = opponentData->getSocket();
      if (result == 0)
      {
        sendMessageToClient(mySocket, "server: Game over. It's a draw");
        sendMessageToClient(opponentSocket, "server: Game over. It's a draw");
      }
      else if ((result > 0 && iAmPlayer1) || (result < 0 && !iAmPlayer1))
      {
        sendMessageToClient(mySocket, "server: Game over. You won");
        sendMessageToClient(opponentSocket, "server: Game over. You lost");
      }
      else
      {
        sendMessageToClient(mySocket, "server: Game over. You lost");
        sendMessageToClient(opponentSocket, "server: Game over. You won");
      }

      // Delete all GameData.
      myData->clearOngoingGame();
      opponentData->clearOngoingGame();
      delete gameData;
    }
  }
  else
    sendMessageToClient(myData->getSocket(),
                        "server: Illegal move %s", position);
}

void GameServer::showBoard(PlayerData* myData)
{
  GameData* gameData;
  if (!getGameData(myData, gameData))
    return;

  // Show board.
  char* boardString = gameData->getBoard()->getBoardAsString();
  sendMessageToClient(myData->getSocket(), "%s", boardString);
  free(boardString);

  // Show if it's your turn to move.
  if (strcmp(myData->getPlayerName(), gameData->getPlayerToMove()) == 0)
    sendMessageToClient(myData->getSocket(), "server: It's your move");
}

void GameServer::resignGame(PlayerData* myData)
{
  GameData* gameData;
  if (!getGameData(myData, gameData))
    return;

  // Send resign confirmation.
  sendMessageToClient(myData->getSocket(), "server: You lost\n");

  // Tell other player we resigned.
  char* myName = myData->getPlayerName();
  char* opponentName = gameData->getOtherPlayer(myName);
  PlayerData* opponentData = getPlayerData(opponentName);
  sendMessageToClient(opponentData->getSocket(),
                      "%s: I resign. You win.", myName);

  // Delete all GameData.
  myData->clearOngoingGame();
  opponentData->clearOngoingGame();
  delete gameData;
}

void GameServer::tellPlayer(PlayerData* myData, char* playerName,
                            const char* message)
{
  PlayerData* otherPlayerData = getPlayerData(playerName);
  if (otherPlayerData == NULL)
  {
    sendMessageToClient(myData->getSocket(),
                        "server: I don't know who %s is", playerName);
    return;
  }
  sendMessageToClient(otherPlayerData->getSocket(), "%s: %s",
                      myData->getPlayerName(), message);
}

void GameServer::sendMessageToClient(int sock, const char* formatString, ...)
{
  if (sock > 0)
  {
    char buffer[8192];
    va_list argList;
    va_start(argList, formatString);
    int length = vsnprintf(buffer, sizeof(buffer), formatString, argList);
    va_end(argList);

    write(sock, buffer, length);
    printf("Message to client (%d): %s\n", sock, buffer);
  }
}

void GameServer::sendGameDataToPlayers(GameData* gameData,
                                       bool showPlayerToMove,
                                       char* lastMovePosition)
{
  // Show previous move for next player to move.
  char* playerToMove = gameData->getPlayerToMove();
  int playerToMoveSocket = getSocket(playerToMove);
  if (lastMovePosition != NULL)
    sendMessageToClient(playerToMoveSocket, "%s: play %s\n",
                        gameData->getOtherPlayer(playerToMove),
                        lastMovePosition);

  // Show resulting game board for both players.
  int player1Socket = getSocket(gameData->getPlayer1());
  int player2Socket = getSocket(gameData->getPlayer2());
  char* boardString = gameData->getBoard()->getBoardAsString();
  sendMessageToClient(player1Socket, "%s", boardString);
  sendMessageToClient(player2Socket, "%s", boardString);
  free(boardString);

  // Show who should move next.
  if (showPlayerToMove)
    sendMessageToClient(playerToMoveSocket, "server: It's your move");
}

bool GameServer::playerExist(char* playerName)
{
  return (getPlayerData(playerName) != NULL);
}

PlayerData* GameServer::getPlayerData(char* playerName)
{
  auto iterator = players.find(playerName);
  return (iterator != players.end() ? iterator->second : NULL);
}

int GameServer::getSocket(char* playerName)
{
  PlayerData* playerData = getPlayerData(playerName);
  return (playerData != NULL ? playerData->getSocket() : -1);
}

bool GameServer::getChallengeData(int sock, char* challenger,
                                  PlayerData*& challengerData,
                                  GameData*& gameData)
{
  challengerData = getPlayerData(challenger);
  if (challengerData == NULL)
  {
    sendMessageToClient(sock, "server: I don't know who %s is", challenger);
    return false;
  }

  // Check if there's a challenge from challenger to accept.
  auto iterator = challenges.find(challenger);
  if (iterator == challenges.end())
  {
    sendMessageToClient(sock, "server: There's no challenge from %s",
                        challenger);
    return false;
  }

  // All data found ok.
  gameData = iterator->second;
  return true;
}

bool GameServer::getGameData(PlayerData* playerData, GameData*& gameData)
{
  // Check if we're playing a game.
  gameData = playerData->getOngoingGame();
  if (gameData != NULL)
    return true;
  else
  {
    sendMessageToClient(playerData->getSocket(),
                        "server: You're not playing any game");
    return false;
  }
}
