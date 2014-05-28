#include "GameBoardFactory.h"
#include "TicTacToeBoard.h"
#include "Connect4Board.h"
//#include "OthelloBoard.h"

using namespace std;

GameBoardFactory* GameBoardFactory::instance = NULL;

GameBoardFactory::GameBoardFactory()
{
  gameBoards["tictactoe"] = new TicTacToeBoard();
  gameBoards["ttt"] = new TicTacToeBoard();   // Alias for tictactoe
  gameBoards["connect4"] = new Connect4Board();
//  gameBoards["othello"] = new OthelloBoard();

  // Build game list string.
  for (auto iter=gameBoards.begin(); iter!=gameBoards.end(); ++iter)
  {
    gameListString += iter->first;
    gameListString += '\n';
  }
}

GameBoardFactory::~GameBoardFactory()
{
  for (auto iter=gameBoards.begin(); iter!=gameBoards.end(); ++iter)
    delete iter->second;
}

GameBoardFactory* GameBoardFactory::getInstance()
{
  if (instance == NULL)
    instance = new GameBoardFactory();
  return instance;
}

const char* GameBoardFactory::getGameList()
{
  return gameListString.c_str();
}

bool GameBoardFactory::gameExists(char* nameOfGame)
{
  return (gameBoards.find(nameOfGame) != gameBoards.end());
}

GameBoard* GameBoardFactory::createGameBoard(char* nameOfGame)
{
  auto iterator = gameBoards.find(nameOfGame);
  return (iterator != gameBoards.end() ? iterator->second->create() : NULL);
}
