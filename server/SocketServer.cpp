#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#include "SocketServer.h"

using namespace std;

SocketServer::SocketServer(int port)
{
  serverPort = port;
  gameServer = new GameServer();
}

SocketServer::~SocketServer()
{
  delete gameServer;
}

void SocketServer::mainLoop()
{
  // Create a socket.
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
  {
    perror("ERROR opening socket");
    exit(1);
  }

  // Initialize socket structure.
  sockaddr_in serverAddress;
  memset((char*)&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = INADDR_ANY;
  serverAddress.sin_port = htons(serverPort);

  // Bind the host address using bind().
  if (::bind(sock, (sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
  {
    perror("ERROR on binding");
    exit(1);
  }

  // Start listening for the clients.
  printf("Game server is up and running on port %d\n", serverPort);
  listen(sock, 100);
  sockaddr_in clientAddress;
  int clientLength = sizeof(clientAddress);
  while (1)
  {
    // Accept connection from client.
    int clientSocket =
      accept(sock, (sockaddr*)&clientAddress, (socklen_t*)&clientLength);
    if (clientSocket < 0)
    {
      perror("ERROR on accept");
      exit(1);
    }

    // Create child thread.
    pthread_t thread;
    ClientThreadData* clientThreadData =
      new ClientThreadData(gameServer, clientSocket);
    pthread_create(&thread, NULL, clientThreadStart, (void*)clientThreadData);
  }
}

void* SocketServer::clientThreadStart(void* clientThreadData)
{
  ClientThreadData* data = (ClientThreadData*)clientThreadData;
  GameServer* server = data->server;
  server->clientThreadMainLoop(data->sock);
  close(data->sock);
  return NULL;
}

