#ifndef TICTACTOE_BOARD_H
#define TICTACTOE_BOARD_H

#include "GameBoard.h"

// Implementation of a TicTacToe board.

class TicTacToeBoard : public GameBoard
{
 public:
  TicTacToeBoard();
  virtual ~TicTacToeBoard();

  virtual GameBoard* create();
  virtual bool makeMove(char* position);  // Returns true if move is legal.
  virtual bool isGameOver() const;
  virtual int getPlayer1Result() const;
  virtual char* getBoardAsString() const;

 private:
  // Private tictactoe specific data goes here.
  unsigned int player1;
  unsigned int player2;
  bool player1ToMove;
};

#endif
