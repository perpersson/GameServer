#ifndef OTHELLO_BOARD_H
#define OTHELLO_BOARD_H

#include "GameBoard.h"

// Implementation of an Othello board.

class OthelloBoard : public GameBoard
{
 public:
  OthelloBoard();
  virtual ~OthelloBoard();

  virtual GameBoard* create();
  virtual bool makeMove(char* position);  // Returns true if move is legal.
  virtual bool isGameOver() const;
  virtual int getPlayer1Result() const;
  virtual char* getPossibleMovesString() const;
  virtual char* getBoardAsString() const;

 private:
  // Private othello specific data goes here.
  unsigned char board[64];   // Index 0 is top-left, then row by row.
  bool player1ToMove;

  void indexToRowAndColumn(int index, int& row, int& column) const
  {
    row = index / 8;
    column = index % 8;
  }
  int getIndexFromRowAndColumn(int row, int column) const
  { return row * 8 + column; }

  bool anyLegalMove(char playerMarker) const;
  bool legalMove(int position, char playerMarker) const;
  bool legalDirection(int position, int dx, int dy,
                      char playerMarker, char otherMarker) const;
  void flipMarkers(int position, int dx, int dy,
                   char playerMarker, char otherMarker);
};

#endif
