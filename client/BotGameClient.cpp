#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BotGameClient.h"

BotGameClient::BotGameClient(const char* hostname, int port,
                             const char* name, const char* game) :
  GameClient(hostname, port, name, game),
  opponentName(NULL), playingGame(false)
{
}

BotGameClient::~BotGameClient()
{
  free(opponentName);
}

void BotGameClient::onWaitForInput()
{
  // Do nothing. It's a bot. Input is allowed though.
}

void BotGameClient::onStdinDataReceived(char* data)
{
  writeToServer(data);
}

void BotGameClient::onServerDataReceived(char* data)
{
  printf("%s\n", data);

  // Parse incoming data to identify some commands that Bot should handle.
  if (strncmp(data, "server:", 7) == 0)
  {
    // server: mrx is challenging you to a game of chess
    char* challengePtr = strstr(data, " is challenging you");
    if (challengePtr != NULL)
    {
      *challengePtr = '\0';  // Terminate string to get opponent name.
      handleChallenge(&data[8]);
    }

    // server: move by mrx: d4
    else if (strncmp(data, "server: move by ", 16) == 0)
    {
      unsigned int moveStringIndex = 18 + strlen(opponentName);
      opponentMove(&data[moveStringIndex]);
    }

    // server: It's your move
    else if (strncmp(data, "server: It's your move", 22) == 0)
      handleBotMove();

    // server: Illegal move %s
    else if (strncmp(data, "server: Illegal move", 20) == 0)
      handleBotMisbehaviour();

    // server: %s resigned. You win
    else if (strstr(data, " resigned. You win") != NULL)
      handleResign();

    // server: Game over. It's a draw
    // server: Game over. You won
    // server: Game over. You lost
    else if (strncmp(data, "server: Game over", 17) == 0)
    {
      int score = 0;
      if (strstr(data, "You won") != NULL)
        score = 1;
      else if (strstr(data, "You lost") != NULL)
        score = -1;
      handleGameOver(score);
    }
  }
}

void BotGameClient::cleanUpGameData()
{
  playingGame = false;
  free(opponentName);
  opponentName = NULL;
}

void BotGameClient::handleBotMisbehaviour()
{
  // Concrete bot subclass isn't behaving. Make it lose.
  writeToServer("resign");
  cleanUpGameData();
}

void BotGameClient::handleChallenge(const char* opponent)
{
  if (playingGame)
    writeToServer("reject %s", opponent);
  else
  {
    // Store opponent name and accept challenge.
    opponentName = strdup(opponent);
    writeToServer("accept %s", opponent);

    // Inform concrete bot subclass that a new game started.
    newGame();
    playingGame = true;
  }
}

void BotGameClient::handleBotMove()
{
  char myPosition[10] = {0};
  decideBotMove(myPosition);
  writeToServer("move %s", myPosition);
}

void BotGameClient::handleResign()
{
  // Inform concrete bot subclass about resignation.
  opponentResigned();
  cleanUpGameData();
}

void BotGameClient::handleGameOver(int score)
{
  // Inform concrete bot subclass about game result.
  gameOver(score);
  cleanUpGameData();
}
