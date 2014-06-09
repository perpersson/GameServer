#ifndef GAME_BOARD_FACTORY_H
#define GAME_BOARD_FACTORY_H

#include <string>
#include <map>
#include "GameBoard.h"
#include "StringCompareFunctor.h"

// GameBoardFactory creates an object of the corresponding GameBoard subclass
// when given a string as input.

class GameBoardFactory
{
 public:
  static GameBoardFactory* getInstance();
  ~GameBoardFactory();

  const char* getGameList();
  const char* getFullNameOfGame(const char* nameOfGameStart);
  GameBoard* createGameBoard(char* nameOfGame);

 private:
  GameBoardFactory();

  static GameBoardFactory* instance;

  std::map<char*, GameBoard*, StringCompareFunctor> gameBoards;
  std::string gameListString;
};

#endif
