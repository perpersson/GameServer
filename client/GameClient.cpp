#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char* argv[])
{
  if (argc < 3)
  {
    fprintf(stderr, "usage %s hostname port [name] [game]\n", argv[0]);
    exit(0);
  }

  char* name = NULL;
  char* game = NULL;
  if (argc >= 3)
    name = argv[3];
  if (argc >= 4)
    game = argv[4];
    

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
  bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr,
        server->h_length);
  serv_addr.sin_port = htons(portno);
  if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
  {
    perror("ERROR connecting");
    exit(1);
  }

  char buffer[8192];

  // Send name and game if given as arguments.
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

  bool commandEntered = true;
  while (1)
  {
    if (commandEntered)
      printf("Enter your command:\n");
    commandEntered = false;

    // Wait for both stdin and socket data.
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(fileno(stdin), &read_fds);
    FD_SET(sock, &read_fds);
    if (select(sock + 1, &read_fds, NULL, NULL, NULL) == -1)
    {
      perror("ERROR in select");
      exit(1);
    }

    // Is there any data to read from the socket?
    if (FD_ISSET(sock, &read_fds))
    {
      memset(buffer, 0, sizeof(buffer));
      int n = read(sock, buffer, sizeof(buffer) - 1);
      if (n < 0) 
      {
        perror("ERROR reading from socket");
        exit(1);
      }
      if (n > 0)
        printf("%s\n", buffer);
    }

    // Is there something from stdin?
    if (FD_ISSET(fileno(stdin), &read_fds))
    {
      commandEntered = true;
      memset(buffer, 0, sizeof(buffer));
      fgets(buffer, sizeof(buffer) - 1, stdin);

      // Send message to server.
      int n = write(sock, buffer, strlen(buffer));
      if (n < 0) 
      {
        perror("ERROR writing to socket");
        exit(1);
      }
    }
  }
  return 0;
}
