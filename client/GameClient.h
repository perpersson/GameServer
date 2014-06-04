#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include "SocketClient.h"

class GameClient : public SocketClient
{
 public:
  GameClient(const char* hostname, int port,
             const char* name = NULL, const char* game = NULL);

 protected:
  // Overridden methods from base class.
  virtual void onServerConnected();
  virtual void onWaitForInput();
  virtual void onStdinDataReceived(char* data, unsigned int dataLength);
  virtual void onServerDataReceived(char* data, unsigned int dataLength);

 private:
  const char* name;
  const char* game;
};

#endif
