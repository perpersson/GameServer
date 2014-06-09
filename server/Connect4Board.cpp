#include <string.h>
#include <ctype.h>
#include "Connect4Board.h"

Connect4Board::Connect4Board()
{
  memset(board, '.', sizeof(board));
  player1ToMove = true;
  moveCount = 0;
}

Connect4Board::~Connect4Board()
{
}

GameBoard* Connect4Board::create()
{
  return new Connect4Board();
}

bool Connect4Board::makeMove(char* position)
{
  // Check for illegal position.
  if (position[0] < '1' || position[0] > '7' || position[1] != '\0')
    return false;

  // Convert to internal row and column values and get next free row.
  unsigned int column = position[0] - '1';
  unsigned int row = 5;
  while (board[getIndexFromRowAndColumn(row, column)] != '.')
  {
    if (row-- == 0)
      return false;   // Column is full.
  }

  // Make the move.
  board[getIndexFromRowAndColumn(row, column)] = (player1ToMove ? 'X' : 'O');
  ++moveCount;
  player1ToMove = !player1ToMove;
  return true;
}

bool Connect4Board::isGameOver() const
{
  // Check for full board or a winner.
  return (moveCount == 42 || getPlayer1Result() != 0);
}

int Connect4Board::getPlayer1Result() const
{
  int result;
  // Check for any row win.
  if (isWinLine(0, 1, 0, result) ||
      isWinLine(7, 1, 0, result) ||
      isWinLine(14, 1, 0, result) ||
      isWinLine(21, 1, 0, result) ||
      isWinLine(28, 1, 0, result) ||
      isWinLine(35, 1, 0, result))
    return result;
  // Check for any column win.
  if (isWinLine(0, 0, 1, result) ||
      isWinLine(1, 0, 1, result) ||
      isWinLine(2, 0, 1, result) ||
      isWinLine(3, 0, 1, result) ||
      isWinLine(4, 0, 1, result) ||
      isWinLine(5, 0, 1, result) ||
      isWinLine(6, 0, 1, result))
    return result;
  // Check for any down-right diagonal win.
  if (isWinLine(0, 1, 1, result) ||
      isWinLine(1, 1, 1, result) ||
      isWinLine(2, 1, 1, result) ||
      isWinLine(3, 1, 1, result) ||
      isWinLine(7, 1, 1, result) ||
      isWinLine(14, 1, 1, result))
    return result;
  // Check for any down-left diagonal win.
  if (isWinLine(3, -1, 1, result) ||
      isWinLine(4, -1, 1, result) ||
      isWinLine(5, -1, 1, result) ||
      isWinLine(6, -1, 1, result) ||
      isWinLine(13, -1, 1, result) ||
      isWinLine(20, -1, 1, result))
    return result;

  return 0;   // It's a draw.
}

char* Connect4Board::getPossibleMovesString() const
{
  char possibleMovesString[2*7+1];
  int stringPos = 0;
  for (int index=0; index<7; ++index)
    if (board[index] == '.')
    {
      possibleMovesString[stringPos++] = index + '1';
      possibleMovesString[stringPos++] = ' ';
    }

  possibleMovesString[stringPos - !!stringPos] = '\0';
  return strdup(possibleMovesString);
}

char* Connect4Board::getBoardAsString() const
{
  const char* emptyBoardString =
    " 1 2 3 4 5 6 7\n . . . . . . .\n . . . . . . .\n . . . . . . .\n"
    " . . . . . . .\n . . . . . . .\n . . . . . . .\n";

  char* boardString = strdup(emptyBoardString);
  for (unsigned int row=0; row<6; ++row)
    for (unsigned int column=0; column<7; ++column)
    {
      unsigned int stringIndex = column * 2 + row * 15 + 16;
      boardString[stringIndex] = board[getIndexFromRowAndColumn(row, column)];
    }

  return boardString;
}

bool Connect4Board::isWinLine(unsigned int startIndex, int dx, int dy,
                              int& result) const
{
  // Return true if we have a winner, and then also set result to +1 or -1.
  unsigned int identicalCount = 0;
  char previousMarker = '.';
  int row, column;
  indexToRowAndColumn(startIndex, row, column);
  while (row < 6 && column < 7)
  {
    int index = getIndexFromRowAndColumn(row, column);
    char thisMarker = board[index];
    if (thisMarker == '.' || thisMarker != previousMarker)
      identicalCount = 1;
    else if (++identicalCount == 4)
    {
      result = (thisMarker == 'X' ? 1 : -1);
      return true;
    }
    previousMarker = thisMarker;
    row += dy;
    column += dx;
  }
  return false;
}
