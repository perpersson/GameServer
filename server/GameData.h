#ifndef GAME_DATA
#define GAME_DATA

#include <stdlib.h>
#include "GameBoard.h"
#include "GameBoardFactory.h"

class GameData
{
 public:
  GameData(char* nameOfGame, char* challenger, char* challengee) :
    nameOfGame(nameOfGame),
    player1(challenger),
    player2(challengee),
    playerToMove(player1),
    board(NULL)
  {}

  void startGame()
  {
    // Create the correct GameBoard and decide who starts playing.
    board = GameBoardFactory::getInstance()->createGameBoard(nameOfGame);
    playerToMove = (rand() % 2 == 0 ? player1 : player2);
  }

  char* getChallenger() const { return player1; }
  char* getChallengee() const { return player2; }
  char* getOtherPlayer(const char* player) const
  { return (player == player1 ? player2 : player1); }
  char* getPlayerToMove() const { return playerToMove; }
  
  void switchPlayerToMove()
  { playerToMove = getOtherPlayer(playerToMove); }

  GameBoard* getBoard() const { return board; }

 private:
  char* nameOfGame;   // tictactoe, connect4, chess, othello, ...
  char* player1;
  char* player2;
  char* playerToMove;

  GameBoard* board;
};

#endif
