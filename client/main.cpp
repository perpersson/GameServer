#include <stdio.h>
#include <stdlib.h>
#include "HumanGameClient.h"
#include "TicTacToeBot.h"

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

  if (argc > 4)
  {
    const char* game = argv[4];
    HumanGameClient client(hostname, port, name, game);
    client.mainLoop();
  }
  else
  {
    TicTacToeBot client(hostname, port, name);
    client.mainLoop();
  }

  return 0;
}
