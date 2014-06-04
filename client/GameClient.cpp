#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GameClient.h"

GameClient::GameClient(const char* hostname, int port,
                       const char* name, const char* game) :
  SocketClient(hostname, port), name(name), game(game)
{
}

void GameClient::onServerConnected()
{
  // Send name and game to server  if given.
  char buffer[1024];
  if (name != NULL)
  {
    sprintf(buffer, "name %s\n", name);
    write(sock, buffer, strlen(buffer));
  }
  if (game != NULL)
  {
    sprintf(buffer, "game %s\n", game);
    write(sock, buffer, strlen(buffer));
  }
}

void GameClient::onWaitForInput()
{
  printf("Enter your command:\n");
}

void GameClient::onStdinDataReceived(char* data, unsigned int dataLength)
{
  // Send message to server.
  int n = write(sock, data, dataLength);
  if (n < 0)
  {
    perror("ERROR writing to socket");
    exit(1);
  }
}

void GameClient::onServerDataReceived(char* data, unsigned int dataLength)
{
  (void)dataLength;
  printf("%s\n", data);
}
