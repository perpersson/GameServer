#include <string.h>
#include <ctype.h>
#include "TicTacToeBoard.h"

TicTacToeBoard::TicTacToeBoard()
{
  player1 = 0;
  player2 = 0;
  player1ToMove = true;
}

TicTacToeBoard::~TicTacToeBoard()
{
}

GameBoard* TicTacToeBoard::create()
{
  return new TicTacToeBoard();
}

bool TicTacToeBoard::makeMove(char* position)
{
  // Check for illegal position.
  position[0] = tolower(position[0]);
  if (position[0] < 'a' || position[0] > 'c' ||
      position[1] < '1' || position[1] > '3')
    return false;

  // Convert to internal position values.
  int internalPosition = position[0] - 'a' + (position[1] - '1') * 3;
  unsigned int positionBit = 1 << internalPosition;

  // Position taken already?
  if (((player1 | player2) & positionBit) != 0)
    return false;

  // Make the move.
  if (player1ToMove)
    player1 |= positionBit;
  else
    player2 |= positionBit;
  player1ToMove = !player1ToMove;
  return true;
}

bool TicTacToeBoard::isGameOver() const
{
  // Check for full board or a winner.
  return ((player1 | player2) == 0x1ff || getPlayer1Result() != 0);
}

int TicTacToeBoard::getPlayer1Result() const
{
  const unsigned int winningCombinations[8] =
    {0x07, 0x38, 0x1c0, 0x49, 0x92, 0x124, 0x54, 0x111};
  for (int index=0; index<8; ++index)
  {
    unsigned int combination = winningCombinations[index];
    if ((player1 & combination) == combination)
      return 1;
    else if ((player2 & combination) == combination)
      return -1;
  }
  return 0;   // It's a draw.
}

char* TicTacToeBoard::getBoardAsString() const
{
  const char* emptyBoardString =
    "   A B C \n  +-+-+-+\n1 | | | |\n  +-+-+-+\n"
    "2 | | | |\n  +-+-+-+\n3 | | | |\n  +-+-+-+\n";
  const int stringIndex[] = {23, 25, 27, 43, 45, 47, 63, 65, 67};

  char* boardString = strdup(emptyBoardString);
  for (unsigned int i=0; i<9; ++i)
  {
    unsigned int position = 1 << i;
    char c;
    if ((player1 & position) != 0)
      c = 'X';
    else if ((player2 & position) != 0)
      c = 'O';
    else
      c = ' ';
    boardString[stringIndex[i]] = c;
  }
  return boardString;
}
