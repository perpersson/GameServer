#ifndef GAME_BOARD_H
#define GAME_BOARD_H

// Generic interface for a board for all types of games.

class GameBoard
{
 public:
  GameBoard() {}
  virtual ~GameBoard() {}

  virtual GameBoard* create() = 0;
  virtual bool makeMove(char* position) = 0;  // Returns true if move is legal.
  virtual bool isGameOver() const = 0;
  virtual int getPlayer1Result() const = 0;
  virtual char* getPossibleMovesString() const = 0;
  virtual char* getBoardAsString() const = 0;
};

#endif
