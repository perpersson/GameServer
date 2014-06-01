#ifndef GAME_DATA_H
#define GAME_DATA_H

#include <stdlib.h>
#include "GameBoard.h"
#include "GameBoardFactory.h"

class GameData
{
 public:
  GameData(char* nameOfGame, char* challenger, char* challengee) :
    nameOfGame(nameOfGame),
    challenger(challenger),
    challengee(challengee),
    player1(challenger),
    player2(challengee),
    playerToMove(player1),
    board(NULL)
  {}
  ~GameData() { delete board; }

  void startGame()
  {
    // Create the correct GameBoard and decide who starts playing.
    board = GameBoardFactory::getInstance()->createGameBoard(nameOfGame);
    if (rand() % 2 == 0)
    {
      player1 = challengee;
      player2 = challenger;
    }
    playerToMove = player1;
  }

  char* getChallenger() const { return challenger; }
  char* getChallengee() const { return challengee; }
  char* getPlayer1() const { return player1; }
  char* getPlayer2() const { return player2; }
  char* getOtherPlayer(const char* player) const
  { return (player == player1 ? player2 : player1); }
  char* getPlayerToMove() const { return playerToMove; }
  
  void switchPlayerToMove()
  { playerToMove = getOtherPlayer(playerToMove); }

  GameBoard* getBoard() const { return board; }

 private:
  char* nameOfGame;   // tictactoe, connect4, chess, othello, ...
  char* challenger;
  char* challengee;
  char* player1;
  char* player2;
  char* playerToMove;

  GameBoard* board;
};

#endif
