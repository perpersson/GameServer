#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include "GameServer.h"

class SocketServer
{
 public:
  SocketServer(int port);
  ~SocketServer();

  void mainLoop();

 private:
  struct ClientThreadData
  {
    ClientThreadData(GameServer* server, int sock) :
      server(server), sock(sock) {}
    GameServer* server;
    int sock;
  };
  static void* clientThreadStart(void* clientThreadData);

  int serverPort;
  GameServer* gameServer;
};

#endif
