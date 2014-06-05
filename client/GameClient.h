#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include <stdlib.h>
#include <sys/types.h>

class GameClient
{
 public:
  GameClient(const char* hostname, int port,
             const char* name, const char* game);

  void mainLoop();

 protected:
  // Methods to override in specific game clients subclasses.
  virtual void onServerConnected() {}
  virtual void onWaitForInput() {}
  virtual void onStdinDataReceived(char* data) = 0;
  virtual void onServerDataReceived(char* data) = 0;

  // Helper methods to be used by subclasses.
  void writeToServer(const char* formatString, ...);

 private:
  void connectToServer(const char* hostname, int port);
  void waitForInput(fd_set* read_fds, bool waitStdin,
                    bool waitSocket, int timeoutInMilliSeconds = 0);

  bool checkForStdinData(fd_set* read_fds);
  bool checkForSocketData(fd_set* read_fds);

  int sock;
  const char* hostname;
  int port;
  const char* name;
  const char* game;

  bool disconnect;
};

#endif
