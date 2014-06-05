#include <stdio.h>
#include <string.h>

#include "HumanGameClient.h"

HumanGameClient::HumanGameClient(const char* hostname, int port,
                                 const char* name, const char* game) :
  GameClient(hostname, port, name, game)
{
}

void HumanGameClient::onWaitForInput()
{
  printf("Enter your command:\n");
}

void HumanGameClient::onStdinDataReceived(char* data)
{
  writeToServer(data);
}

void HumanGameClient::onServerDataReceived(char* data)
{
  printf("%s\n", data);
}
