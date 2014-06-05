#ifndef HUMAN_GAME_CLIENT_H
#define HUMAN_GAME_CLIENT_H

#include "GameClient.h"

class HumanGameClient : public GameClient
{
 public:
  HumanGameClient(const char* hostname, int port,
                  const char* name, const char* game);

 protected:
  // Overridden methods from base class.
  virtual void onWaitForInput();
  virtual void onStdinDataReceived(char* data);
  virtual void onServerDataReceived(char* data);
};

#endif
