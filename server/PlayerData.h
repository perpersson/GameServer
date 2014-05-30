#ifndef PLAYER_DATA_H
#define PLAYER_DATA_H

#include <stdlib.h>
#include <string.h>
#include "GameData.h"

class PlayerData
{
 public:
  PlayerData(char* playerName, int sock);
  ~PlayerData();

  void startGame(char* opponent);

  int getSocket() { return sock; }
  char* getPlayerName() { return playerName; }
  char* getFavouriteGame() { return favouriteGame; }
  void setFavouriteGame(char* newFavouriteGame)
  {
    free(favouriteGame);
    favouriteGame = strdup(newFavouriteGame);
  }
  bool isPlayingAGame() const { return (onGoingGame != NULL); }
  GameData* getOngoingGame() { return onGoingGame; }
  void setOngoingGame(GameData* game) { onGoingGame = game; }
  void clearOngoingGame() { onGoingGame = NULL; }

 private:
  int sock;
  char* playerName;
  char* favouriteGame;   // tictactoe, connect4, chess, othello, ...

  GameData* onGoingGame;
};

#endif
