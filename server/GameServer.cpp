#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#include "GameServer.h"
#include "GameBoard.h"
#include "TicTacToeBoard.h"

using namespace std;

GameServer::GameServer()
{
  commandHandler = CommandHandler::getInstance();
}

void GameServer::mainLoop()
{
  // Create a socket.
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
    perror("ERROR opening socket");
    exit(1);
  }

  // Initialize socket structure.
  int portno = 6792;
  sockaddr_in serverAddress;
  memset((char*)&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = INADDR_ANY;
  serverAddress.sin_port = htons(portno);

  // Bind the host address using bind().
  if (bind(sockfd, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
  {
    perror("ERROR on binding");
    exit(1);
  }

  // Start listening for the clients.
  printf("Game server is up and running on port %d\n", portno);
  listen(sockfd, 100);
  sockaddr_in clientAddress;
  int clientLength = sizeof(clientAddress);
  while (1)
  {
    // Accept connection from client.
    int newsockfd =
      accept(sockfd, (sockaddr*)&clientAddress, (socklen_t*)&clientLength);
    if (newsockfd < 0)
    {
      perror("ERROR on accept");
      exit(1);
    }

    // Create child thread.
    pthread_t thread;
    ClientThreadData* clientThreadData = new ClientThreadData(this, newsockfd);
    pthread_create(&thread, NULL, clientThreadStart, (void*)clientThreadData);
  }
}

void* GameServer::clientThreadStart(void* clientThreadData)
{
  ClientThreadData* data = (ClientThreadData*)clientThreadData;
  GameServer* server = data->server;
  server->clientThreadMainLoop(data->sock);
  return NULL;
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
    switch (command)
    {
      case NoDataCommand:
        break;
      case NameNotGivenYetCommand:
        sendMessageToClient(sock, "Error: Tell me your name first");
        break;

      // Informational commands
      case HelpCommand:       showCommands(sock); break;
      case PlayersCommand:    showPlayers(sock); break;
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
        sendMessageToClient(sock, "Error: Unknown command '%s'", buffer);
        break;
    }
  }
}

void GameServer::showCommands(int sock)
{
  const char* commands =
    "help                  Show this help\n"
    "players               List all players\n"
    "games                 List all games that are possible to play\n"
    "name <your name>      Set your name\n"
    "game <game to play>   Set the game you want to play\n"
    "challenge <player>    Challenge another player\n"
    "recall <player>       Recall your challenge\n"
    "accept <player>       Accept challenge from another player\n"
    "reject <player>       Reject challenge from another player\n"
    "play <move>           Make a move in the ongoing game\n"
    "resign                Give up current game\n"
    "tell <player> <msg>   Send a message to player\n";

  sendMessageToClient(sock, "%s", commands);
}

void GameServer::showPlayers(int sock)
{
  char buffer[8192];
  int len = sprintf(buffer, "The players are:\n");
  for (auto iterator=players.begin(); iterator!=players.end(); ++iterator)
  {
    len += sprintf(&buffer[len], "%-16s: ", iterator->first);

    // Check what game player is playing or wants to play.
    PlayerData* playerData = iterator->second;
    char* game = playerData->getFavouriteGame();
    if (game == NULL)
      len += sprintf(&buffer[len], "Game not selected\n");
    else
      len += sprintf(&buffer[len], "%s%s\n", game,
                     playerData->isPlayingAGame() ? " (playing)" : "");
  }
  sendMessageToClient(sock, "%s", buffer);
}

void GameServer::showGames(int sock)
{
  // Ask GameBordFactory for all known games.
  sendMessageToClient(sock, "%s",
                      GameBoardFactory::getInstance()->getGameList());
}

void GameServer::showChallenges(PlayerData* myData)
{
}

void GameServer::addPlayer(char* playerName, int sock, PlayerData*& playerData)
{
  printf("Adding player %s\n", playerName);

  if (playerData != NULL)
  {
    sendMessageToClient(sock, "You can't change your name %s",
                        playerData->getPlayerName());
    return;
  }

//...no spaces in name!...


  if (playerExist(playerName))
  {
    sendMessageToClient(sock, "Player %s already exists", playerName);
    return;
  }

  // Create new PlayerData and store name and socket.
  playerData = new PlayerData(playerName, sock);
  players[strdup(playerName)] = playerData;
}

void GameServer::setFavouriteGame(PlayerData* myData, char* nameOfGame)
{
  // Validate that we know the rules of the selected game.
  if (GameBoardFactory::getInstance()->gameExists(nameOfGame))
    myData->setFavouriteGame(nameOfGame);
  else
    sendMessageToClient(myData->getSocket(),
                        "Error: I don't know the rules of %s", nameOfGame);
}

void GameServer::challengeOtherPlayer(PlayerData* myData, char* challengee)
{
  // Get data for other player.
  PlayerData* opponentData = getPlayerData(challengee);
  if (opponentData == NULL)
  {
    sendMessageToClient(myData->getSocket(),
                        "Error: Don't know who %s is", challengee);
    return;
  }

  // Player must be another client.
  if (myData == opponentData)
  {
    sendMessageToClient(myData->getSocket(),
                        "Error: You're not allowed to challenge yourself");
    return;
  }

  // Ensure a favourite game has been selected.
  if (myData->getFavouriteGame() == NULL)
  {
    sendMessageToClient(myData->getSocket(),
                        "Error: Tell me your favourite game first");
    return;
  }

  // Ensure other player wants to play the same game.
  if (strcmp(myData->getFavouriteGame(),
             opponentData->getFavouriteGame()) != 0)
  {
    sendMessageToClient(myData->getSocket(), "Error: %s wants to play %s",
                        challengee, opponentData->getFavouriteGame());
    return;
  }

  // Check that we're not already playing a game.
  char* challenger = myData->getPlayerName();
  GameData* gameData = myData->getOngoingGame();
  if (gameData != NULL)
  {
    sendMessageToClient(myData->getSocket(),
                        "Error: You're already playing a game against %s",
                        gameData->getOtherPlayer(challenger));
    return;
  }

  // Check that challengee isn't already playing a game.
  gameData = opponentData->getOngoingGame();
  if (gameData != NULL)
  {
    sendMessageToClient(myData->getSocket(),
                        "Error: %s is already playing a game against %s",
                        challengee, gameData->getOtherPlayer(challengee));
    return;
  }

  // Check that we haven't sent another challenge.
  auto iterator = challenges.find(challenger);
  if (iterator != challenges.end())
  {
    sendMessageToClient(myData->getSocket(),
                        "Error: You have already challenged %s",
                        iterator->second->getOtherPlayer(challenger));
    return;
  }

  // Create and store new challenge.
  challenges[challenger] = new GameData(myData->getFavouriteGame(),
                                        myData->getPlayerName(),
                                        opponentData->getPlayerName());

  // Send challenge to other player.
  sendMessageToClient(opponentData->getSocket(),
                      "%s is challenging you to a game of %s",
                      myData->getPlayerName(),
                      myData->getFavouriteGame());
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
                        "%s is recalling the challenge to a game of %s",
                        myData->getPlayerName(),
                        myData->getFavouriteGame());

    delete iterator->second;
    challenges.erase(challenger);
  }
  else
  {
    sendMessageToClient(myData->getSocket(),
                        "Error: You haven't challenged %s", challengee);
  }
}

void GameServer::acceptChallenge(PlayerData* myData, char* challenger)
{
  // Check if there's a game ongoing already.
  if (myData->isPlayingAGame())
  {
    sendMessageToClient(myData->getSocket(),
                        "Error: You're already playing a game");
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
                          "Info: Recalling your challenge to %s\n",
                          iterator->second->getChallengee());
      recallChallenge(myData, iterator->second->getChallengee());
    }

    // Remove the challenge from challenges map.
    challenges.erase(challenger);

    // Send challenge accepted to other player.
    sendMessageToClient(opponentData->getSocket(),
                        "%s accepted the challenge of a game of %s\n",
                        myData->getPlayerName(),
                        opponentData->getFavouriteGame());

    // Start the game and tell the players who should start playing.
    myData->setOngoingGame(gameData);
    opponentData->setOngoingGame(gameData);

    gameData->startGame();
    sendGameBoardToPlayers(gameData);
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
                        "%s rejected your challenge of a game of %s",
                        myData->getPlayerName(),
                        opponentData->getFavouriteGame());


    // Remove all challenge data.
    delete gameData;
    challenges.erase(challenger);
  }
}

void GameServer::makePlayerMove(PlayerData* myData, char* position)
{
  // Get the board and make the move.
  GameData* gameData = myData->getOngoingGame();
  if (gameData == NULL)
  {
    sendMessageToClient(myData->getSocket(),
                        "Error: You're not playing any game");
    return;
  }

  // Ensure that it's my move.
  if (strcmp(gameData->getPlayerToMove(), myData->getPlayerName()) != 0)
  {
    sendMessageToClient(myData->getSocket(), "Error: It's %s to play",
                        gameData->getPlayerToMove());
    return;
  }

  GameBoard* gameBoard = gameData->getBoard();
  if (gameBoard->makeMove(position))
  {
    // Switch player to move and send board to both players.
    gameData->switchPlayerToMove();
    sendGameBoardToPlayers(gameData);

    if (gameBoard->isGameOver())
    {
      // Send game result to both players.
      int result = gameBoard->getPlayer1Result();
      PlayerData* opponentData = getPlayerData(gameData->getPlayerToMove());

      //...check correct winner...
      if (result > 0 /*&&
                       strcmp(myData->getPlayerName(), player1) == 0*/)
      {
        sendMessageToClient(myData->getSocket(), "You won");
        sendMessageToClient(opponentData->getSocket(), "You lost");
      }
      else if (result == 0)
      {
        sendMessageToClient(myData->getSocket(), "It's a draw");
        sendMessageToClient(opponentData->getSocket(), "It's a draw");
      }
      else
      {
        sendMessageToClient(myData->getSocket(), "You lost");
        sendMessageToClient(opponentData->getSocket(), "You won");
      }

      printf("Game over. Result: %d\n", result);

      // Remove GameData from PlayerData.
      myData->clearOngoingGame();
      opponentData->clearOngoingGame();
      delete gameData;
      delete gameBoard;
    }
  }
  else
    sendMessageToClient(myData->getSocket(),
                        "Error: Illegal move %s", position);
}

void GameServer::resignGame(PlayerData* myData)
{
}

void GameServer::tellPlayer(PlayerData* myData, char* playerName,
                            const char* message)
{
  PlayerData* otherPlayerData = getPlayerData(playerName);
  if (otherPlayerData == NULL)
  {
    //...don't know who %s is...
    return;
  }

// ...parse out name from message...


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
    printf("Message to client (%d):\n%s\n", sock, buffer);
  }
}

void GameServer::sendGameBoardToPlayers(GameData* gameData)
{
  int player1Socket = getSocket(gameData->getChallenger());
  int player2Socket = getSocket(gameData->getChallengee());
  int playerToMoveSocket = getSocket(gameData->getPlayerToMove());

  char* boardString = gameData->getBoard()->getBoardAsString();
  sendMessageToClient(player1Socket, "%s", boardString);
  sendMessageToClient(player2Socket, "%s", boardString);
  free(boardString);

  sendMessageToClient(playerToMoveSocket, "%s: It's your move",
                      gameData->getPlayerToMove());
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
    sendMessageToClient(sock, "Error: Don't know who %s is", challenger);
    return false;
  }

  // Check if there's a challenge from challenger to accept.
  auto iterator = challenges.find(challenger);
  if (iterator == challenges.end())
  {
    sendMessageToClient(sock, "Error: There's no challenge from %s",
                        challenger);
    return false;
  }

  // All data found ok.
  gameData = iterator->second;
  return true;
}
