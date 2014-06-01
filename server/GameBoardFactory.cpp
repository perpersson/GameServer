#include <string.h>
#include "GameBoardFactory.h"
#include "TicTacToeBoard.h"
#include "Connect4Board.h"
//#include "OthelloBoard.h"

using namespace std;

GameBoardFactory* GameBoardFactory::instance = NULL;

GameBoardFactory::GameBoardFactory()
{
  // Create game boards for all known games.
  gameBoards["tictactoe"] = new TicTacToeBoard();
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

const char* GameBoardFactory::getFullNameOfGame(const char* nameOfGameStart)
{
  unsigned int nameLength = strlen(nameOfGameStart);
  const char* bestMatch = NULL;
  for (auto iter=gameBoards.begin(); iter!=gameBoards.end(); ++iter)
  {
    // Matches beginning of a command?
    if (strncmp(nameOfGameStart, iter->first, nameLength) == 0)
    {
      // Exact match?
      if (strlen(iter->first) == nameLength)
        return nameOfGameStart;
      // Another match already making this one ambigous?
      else if (bestMatch != NULL)
        return NULL;
      // Store best match so far.
      else
        bestMatch = iter->first;
    }
  }
  return bestMatch;
}

GameBoard* GameBoardFactory::createGameBoard(char* nameOfGame)
{
  auto iterator = gameBoards.find(nameOfGame);
  return (iterator != gameBoards.end() ? iterator->second->create() : NULL);
}
