#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "SocketClient.h"

SocketClient::SocketClient(const char* hostname, int port) :
  hostname(hostname), port(port)
{
}

void SocketClient::mainLoop()
{
  connectToServer(hostname, port);

  while (1)
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

void SocketClient::connectToServer(const char* hostname, int port)
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
  onServerConnected();
}

void SocketClient::waitForInput(fd_set* read_fds, bool waitStdin,
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

bool SocketClient::checkForStdinData(fd_set* read_fds)
{
  // Is there something from stdin?
  if (FD_ISSET(fileno(stdin), read_fds))
  {
    char buffer[1024] = {0};
    fgets(buffer, sizeof(buffer) - 1, stdin);
    unsigned int stringLength = strlen(buffer);
    onStdinDataReceived(buffer, stringLength + 1);
    return true;
  }
  return false;
}

bool SocketClient::checkForSocketData(fd_set* read_fds)
{
  // Is there any data to read from the socket?
  if (FD_ISSET(sock, read_fds))
  {
    char buffer[8192] = {0};
    int n = read(sock, buffer, sizeof(buffer) - 1);
    if (n < 0)
    {
      perror("ERROR reading from socket");
      exit(1);
    }
    if (n > 0)
      onServerDataReceived(buffer, n);      
    return true;
  }
  return false;
}
