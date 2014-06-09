#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "GameClient.h"

const char* ServerGoodbyeString = "server: Goodbye";

GameClient::GameClient(const char* hostname, int port,
                       const char* name, const char* game) :
  hostname(hostname), port(port), name(name), game(game)
{
}

void GameClient::mainLoop()
{
  connectToServer(hostname, port);

  disconnect = false;
  while (!disconnect)
  {
    // Wait for both stdin and socket data.
    onWaitForInput();

    fd_set read_fds;
    waitForInput(&read_fds, true, true);
    checkForSocketData(&read_fds);
    if (checkForStdinData(&read_fds))
    {
      // Wait for a while for server response.
      waitForInput(&read_fds, false, true, 500);
      checkForSocketData(&read_fds);
    }
  }
}

void GameClient::writeToServer(const char* formatString, ...)
{
  char buffer[1024];
  va_list argList;
  va_start(argList, formatString);
  int length = vsnprintf(buffer, sizeof(buffer), formatString, argList);
  va_end(argList);
  write(sock, buffer, length);
}

void GameClient::connectToServer(const char* hostname, int port)
{
  // Create a socket point.
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
  {
    perror("ERROR opening socket");
    exit(1);
  }
  hostent* server = gethostbyname(hostname);
  if (server == NULL)
  {
    perror("ERROR, no such host");
    exit(1);
  }

  // Connect to the server.
  sockaddr_in serv_addr;
  memset((char*)&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  memcpy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr,
         server->h_length);
  serv_addr.sin_port = htons(port);
  if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
  {
    perror("ERROR connecting");
    exit(1);
  }

  // Send name and game to server if given.
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

  onServerConnected();
}

void GameClient::waitForInput(fd_set* read_fds, bool waitStdin,
                              bool waitSocket, int timeoutInMilliSeconds)
{
  FD_ZERO(read_fds);
  if (waitStdin)
    FD_SET(fileno(stdin), read_fds);
  if (waitSocket)
    FD_SET(sock, read_fds);
  struct timeval timeout =
    {timeoutInMilliSeconds / 1000, (timeoutInMilliSeconds % 1000) * 1000};
  if (select(sock + 1, read_fds, NULL, NULL,
             (timeoutInMilliSeconds > 0 ? &timeout : NULL)) == -1)
  {
    perror("ERROR in select");
    exit(1);
  }
}

bool GameClient::checkForStdinData(fd_set* read_fds)
{
  // Is there something from stdin?
  if (FD_ISSET(fileno(stdin), read_fds))
  {
    char buffer[1024] = {0};
    fgets(buffer, sizeof(buffer) - 1, stdin);
    onStdinDataReceived(buffer);
    return true;
  }
  return false;
}

bool GameClient::checkForSocketData(fd_set* read_fds)
{
  // Is there any data to read from the socket?
  if (FD_ISSET(sock, read_fds))
  {
    char buffer[8192] = {0};
    if (read(sock, buffer, sizeof(buffer) - 1) == 0)
      disconnect = true;
    else
    {
      char* allData = buffer;
      while (allData != NULL && *allData != '\0')
      {
        // Split input into separate lines.
        char* moreData = strchr(allData, '\n');
        if (moreData != NULL)
          *moreData++ = '\0';
        char* line = allData;
        allData = moreData;

        onServerDataReceived(line);

        // Is server saying Goodbye?
        disconnect = (strncmp(line, ServerGoodbyeString,
                              strlen(ServerGoodbyeString)) == 0);
      }
      return true;
    }
  }
  return false;
}
