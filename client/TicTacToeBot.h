#ifndef TICTACTOE_BOT_H
#define TICTACTOE_BOT_H

#include "BotGameClient.h"
#include "TicTacToeComputer.h"

class TicTacToeBot : public BotGameClient
{
 public:
  TicTacToeBot(const char* hostname, int port, const char* name);
  ~TicTacToeBot();

 protected:
  // Overridden methods from BotGameClient.
  void newGame();
  void opponentMove(const char* position);
  void decideBotMove(char* position);
  void opponentResigned();
  void gameOver(int botScore);

 private:
  TicTacToeComputer* computerPlayer;

  int getComputerPositionFromBotPosition(const char* position) const;
  void getBotPositionFromComputerPosition(int computerPosition,
                                          char* position) const;
};

#endif
