#ifndef CONNECT4_BOARD_H
#define CONNECT4_BOARD_H

#include "GameBoard.h"

// Implementation of a Connect4 board.

class Connect4Board : public GameBoard
{
 public:
  Connect4Board();
  virtual ~Connect4Board();

  virtual GameBoard* create();
  virtual bool makeMove(char* position);  // Returns true if move is legal.
  virtual bool isGameOver() const;
  virtual int getPlayer1Result() const;
  virtual char* getBoardAsString() const;

 private:
  // Private connect4 specific data goes here.
  unsigned char board[42];   // Index 0 is top-left, then row by row.
  unsigned int moveCount;
  bool player1ToMove;

  void indexToRowAndColumn(int index, int& row, int& column) const
  {
    row = index / 7;
    column = index % 7;
  }
  int getIndexFromRowAndColumn(int row, int column) const
  { return row * 7 + column; }

  bool isWinLine(unsigned int startIndex, unsigned int step,
                 int& result) const;
};

#endif
