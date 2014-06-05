#ifndef BOT_GAME_CLIENT_H
#define BOT_GAME_CLIENT_H

#include "GameClient.h"

class BotGameClient : public GameClient
{
 public:
  BotGameClient(const char* hostname, int port,
                const char* name, const char* game);
  virtual ~BotGameClient();

 protected:
  // Overridden methods from base class.
  virtual void onWaitForInput();
  virtual void onStdinDataReceived(char* data);
  virtual void onServerDataReceived(char* data);

  // Methods to be overridden by BotGameClient subclasses.
  virtual void newGame() = 0;
  virtual void opponentMove(const char* position) = 0;
  virtual void decideBotMove(char* position) = 0;
  virtual void opponentResigned() = 0;
  virtual void gameOver(int botScore) = 0;

 protected:
  char* opponentName;

 private:
  bool playingGame;

  void cleanUpGameData();

  // Bot command handling methods.
  void handleBotMisbehaviour();
  void handleChallenge(const char* opponent);
  void handleBotMove();
  void handleResign();
  void handleGameOver(int score);
};

#endif
