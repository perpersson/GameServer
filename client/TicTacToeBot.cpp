#include <stdlib.h>
#include <ctype.h>
#include "TicTacToeBot.h"

TicTacToeBot::TicTacToeBot(const char* hostname, int port, const char* name) :
  BotGameClient(hostname, port, name, "tictactoe"),
  computerPlayer(NULL)
{
}

TicTacToeBot::~TicTacToeBot()
{
  delete computerPlayer;
}

void TicTacToeBot::newGame()
{
  computerPlayer = new TicTacToeComputer();
}

void TicTacToeBot::opponentMove(const char* position)
{
  unsigned int computerPosition =
    getComputerPositionFromBotPosition(position);
  computerPlayer->makeMove(computerPosition);
}

void TicTacToeBot::decideBotMove(char* position)
{
  int computerPosition = computerPlayer->makeComputerMove();
  getBotPositionFromComputerPosition(computerPosition, position);
}

void TicTacToeBot::opponentResigned()
{
  delete computerPlayer;
  computerPlayer = NULL;
}

void TicTacToeBot::gameOver(int botScore)
{
  (void)botScore;    // We don't care about who won.
  delete computerPlayer;
  computerPlayer = NULL;
}

int TicTacToeBot::getComputerPositionFromBotPosition(const char* position) const
{
  int column = tolower(position[0]) - 'a';
  int row = position[1] - '1';
  return row * 3 + column;
}

void TicTacToeBot::getBotPositionFromComputerPosition(int computerPosition,
                                                      char* position) const
{
  int column = computerPosition % 3;
  int row = computerPosition / 3;
  position[0] = column + 'A';
  position[1] = row + '1';
}
