#ifndef GAME_BOARD_FACTORY
#define GAME_BOARD_FACTORY

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

  const char* getGameList();
  bool gameExists(char* nameOfGame);
  GameBoard* createGameBoard(char* nameOfGame);

 private:
  GameBoardFactory();
  ~GameBoardFactory();

  static GameBoardFactory* instance;

  std::map<char*, GameBoard*, StringCompareFunctor> gameBoards;
  std::string gameListString;
};

#endif
