#include <stdio.h>
#include <stdlib.h>
#include "GameClient.h"

int main(int argc, char* argv[])
{
  if (argc < 3)
  {
    fprintf(stderr, "usage %s hostname port [name [game]]\n", argv[0]);
    exit(0);
  }

  const char* hostname = argv[1];
  int port = atoi(argv[2]);
  const char* name = (argc > 3 ? argv[3] : NULL);
  const char* game = (argc > 4 ? argv[4] : NULL);
  GameClient client(hostname, port, name, game);
  client.mainLoop();
  return 0;
}
