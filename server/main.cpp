#include <stdio.h>
#include "SocketServer.h"

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    fprintf(stderr, "usage %s port\n", argv[0]);
    exit(0);
  }

  int port = atoi(argv[1]);
  SocketServer server(port);
  server.mainLoop();
  return 0;
}
