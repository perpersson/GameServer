#include <stdlib.h>
#include "PlayerData.h"

PlayerData::PlayerData(char* playerName, int sock) :
  sock(sock),
  playerName(strdup(playerName)),
  favouriteGame(NULL),
  onGoingGame(NULL)
{
}

PlayerData::~PlayerData()
{
  free(playerName);
  free(favouriteGame);
  delete onGoingGame;
}
