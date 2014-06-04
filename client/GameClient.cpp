#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void waitForInput(int sock, fd_set* read_fds, bool waitStdin, bool waitSocket,
                  int timeoutInMilliSeconds = 0)
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

bool checkForStdinData(int sock, fd_set* read_fds)
{
  // Is there something from stdin?
  if (FD_ISSET(fileno(stdin), read_fds))
  {
    char buffer[8192] = {0};
    fgets(buffer, sizeof(buffer) - 1, stdin);
    
    // Send message to server.
    int n = write(sock, buffer, strlen(buffer));
    if (n < 0)
    {
      perror("ERROR writing to socket");
      exit(1);
    }
    return true;
  }
  return false;
}

bool checkForSocketData(int sock, fd_set* read_fds)
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
      printf("%s\n", buffer);
    return true;
  }
  return false;
}

int main(int argc, char* argv[])
{
  if (argc < 3)
  {
    fprintf(stderr, "usage %s hostname port [name [game]]\n", argv[0]);
    exit(0);
  }

  // Create a socket point.
  int portno = atoi(argv[2]);
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
  {
    perror("ERROR opening socket");
    exit(1);
  }
  hostent* server = gethostbyname(argv[1]);
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
  serv_addr.sin_port = htons(portno);
  if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
  {
    perror("ERROR connecting");
    exit(1);
  }

  // Send name and game if given as arguments.
  char buffer[8192];
  if (argc >= 3)
  {
    sprintf(buffer, "name %s\n", argv[3]);
    write(sock, buffer, strlen(buffer));
  }
  if (argc >= 4)
  {
    sprintf(buffer, "game %s\n", argv[4]);
    write(sock, buffer, strlen(buffer));
  }

  while (1)
  {
    printf("Enter your command:\n");

    // Wait for both stdin and socket data.
    fd_set read_fds;
    waitForInput(sock, &read_fds, true, true);
    checkForSocketData(sock, &read_fds);
    if (checkForStdinData(sock, &read_fds))
    {
      // Wait for a while for server response.
      waitForInput(sock, &read_fds, false, true, 500);
      checkForSocketData(sock, &read_fds);
    }
  }
  return 0;
}
