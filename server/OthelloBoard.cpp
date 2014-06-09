#include <string.h>
#include <ctype.h>
#include "OthelloBoard.h"

OthelloBoard::OthelloBoard()
{
  memset(board, '.', sizeof(board));
  board[28] = board[35] = 'X';
  board[27] = board[36] = 'O';
  player1ToMove = true;
}

OthelloBoard::~OthelloBoard()
{
}

GameBoard* OthelloBoard::create()
{
  return new OthelloBoard();
}

bool OthelloBoard::makeMove(char* position)
{
  // Is it a forced pass move?
  char playerMarker = (player1ToMove ? 'X' : 'O');
  if (strcmp(position, "pass") == 0)
  {
    if (anyLegalMove(playerMarker))
      return false;
    player1ToMove = !player1ToMove;
    return true;
  }

  // Check for illegal position.
  position[0] = tolower(position[0]);
  if (position[0] < 'a' || position[0] > 'h' ||
      position[1] < '1' || position[1] > '8' ||
      position[2] != '\0')
    return false;

  // Convert to internal position values.
  int internalPosition =
    getIndexFromRowAndColumn(position[1] - '1', position[0] - 'a');

  if (!legalMove(internalPosition, playerMarker))
    return false;

  // Make the move.
  char otherMarker = (playerMarker == 'X' ? 'O' : 'X');
  flipMarkers(internalPosition, -1, -1, playerMarker, otherMarker);
  flipMarkers(internalPosition, 0, -1, playerMarker, otherMarker);
  flipMarkers(internalPosition, 1, -1, playerMarker, otherMarker);
  flipMarkers(internalPosition, -1, 0, playerMarker, otherMarker);
  flipMarkers(internalPosition, 1, 0, playerMarker, otherMarker);
  flipMarkers(internalPosition, -1, 1, playerMarker, otherMarker);
  flipMarkers(internalPosition, 0, 1, playerMarker, otherMarker);
  flipMarkers(internalPosition, 1, 1, playerMarker, otherMarker);

  board[internalPosition] = playerMarker;
  player1ToMove = !player1ToMove;
  return true;
}

bool OthelloBoard::isGameOver() const
{
  // It's game over when there's no more legal moves for any player.
  return (!anyLegalMove('X') && !anyLegalMove('O'));
}

int OthelloBoard::getPlayer1Result() const
{
  unsigned int player1Squares = 0;
  unsigned int player2Squares = 0;
  for (unsigned int index=0; index<64; ++index)
    switch (board[index])
    {
      case 'X': ++player1Squares;
      case 'O': ++player2Squares;
    }

  return player1Squares - player2Squares;
}

char* OthelloBoard::getPossibleMovesString() const
{
  char possibleMovesString[3*60+1];
  int stringPos = 0;
  char playerMarker = (player1ToMove ? 'X' : 'O');
  for (unsigned int position=0; position<64; ++position)
    if (legalMove(position, playerMarker))
    {
      int row, column;
      indexToRowAndColumn(position, row, column);
      possibleMovesString[stringPos++] = column + 'a';
      possibleMovesString[stringPos++] = row + '1';
      possibleMovesString[stringPos++] = ' ';
    }

  if (stringPos == 0)
    return strdup("pass");
  possibleMovesString[stringPos - 1] = '\0';
  return strdup(possibleMovesString);
}

char* OthelloBoard::getBoardAsString() const
{
  const char* emptyBoardString =
    "  a b c d e f g h\n1 . . . . . . . .\n2 . . . . . . . .\n"
    "3 . . . . . . . .\n4 . . . . . . . .\n5 . . . . . . . .\n"
    "6 . . . . . . . .\n7 . . . . . . . .\n8 . . . . . . . .\n";

  char* boardString = strdup(emptyBoardString);
  for (unsigned int row=0; row<8; ++row)
    for (unsigned int column=0; column<8; ++column)
    {
      unsigned int stringIndex = column * 2 + row * 18 + 20;
      boardString[stringIndex] = board[getIndexFromRowAndColumn(row, column)];
    }

  return boardString;
}

bool OthelloBoard::anyLegalMove(char playerMarker) const
{
  for (int position=0; position<64; ++position)
    if (legalMove(position, playerMarker))
      return true;

  return false;
}

bool OthelloBoard::legalMove(int position, char playerMarker) const
{
  // Position taken already?
  if (board[position] != '.')
    return false;

  // Check if a move is possible in any direction.
  char otherMarker = (playerMarker == 'X' ? 'O' : 'X');
  return (legalDirection(position, -1, -1, playerMarker, otherMarker) ||
          legalDirection(position, 0, -1, playerMarker, otherMarker) ||
          legalDirection(position, 1, -1, playerMarker, otherMarker) ||
          legalDirection(position, -1, 0, playerMarker, otherMarker) ||
          legalDirection(position, 1, 0, playerMarker, otherMarker) ||
          legalDirection(position, -1, 1, playerMarker, otherMarker) ||
          legalDirection(position, 0, 1, playerMarker, otherMarker) ||
          legalDirection(position, 1, 1, playerMarker, otherMarker));
}

bool OthelloBoard::legalDirection(int position, int dx, int dy,
                                  char playerMarker, char otherMarker) const
{
  int row, column;
  indexToRowAndColumn(position, row, column);
  row += dy;
  column += dx;
  unsigned int opponentMarkers = 0;
  while (row >= 0 && row < 8 && column >= 0 && column < 8 &&
         board[getIndexFromRowAndColumn(row, column)] == otherMarker)
  {
    ++opponentMarkers;
    row += dy;
    column += dx;
  }
  return (opponentMarkers > 0 &&
          row >= 0 && row < 8 && column >= 0 && column < 8 &&
          board[getIndexFromRowAndColumn(row, column)] == playerMarker);
}

void OthelloBoard::flipMarkers(int position, int dx, int dy,
                               char playerMarker, char otherMarker)
{
  int row, column;
  indexToRowAndColumn(position, row, column);
  row += dy;
  column += dx;
  unsigned int opponentMarkers = 0;
  while (row >= 0 && row < 8 && column >= 0 && column < 8 &&
         board[getIndexFromRowAndColumn(row, column)] == otherMarker)
  {
    ++opponentMarkers;
    row += dy;
    column += dx;
  }

  position = getIndexFromRowAndColumn(row, column);
  if (opponentMarkers > 0 &&
      row >= 0 && row < 8 && column >= 0 && column < 8 &&
      board[position] == playerMarker)
  {
    position -= dy * 8 + dx;
    while (board[position] != '.')
    {
      board[position] = playerMarker;
      position -= dy * 8 + dx;
    }
  }
}
