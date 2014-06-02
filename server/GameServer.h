#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include <map>
#include <string.h>
#include "CommandHandler.h"
#include "GameBoardFactory.h"
#include "PlayerData.h"
#include "StringCompareFunctor.h"

class GameServer
{
 public:
  GameServer(int port);

  void mainLoop();

 private:
  struct ClientThreadData
  {
    ClientThreadData(GameServer* server, int sock) :
      server(server), sock(sock) {}
    GameServer* server;
    int sock;
  };
  static void* clientThreadStart(void* clientThreadData);
  void clientThreadMainLoop(int sock);

  // Client command methods.
  void showCommands(int sock);
  void showPlayers(int sock);
  void showGames(int sock);
  void showChallenges(PlayerData* myData);

  void addPlayer(char* playerName, int sock, PlayerData*& playerData);
  void setFavouriteGame(PlayerData* myData, char* game);

  void challengeOtherPlayer(PlayerData* myData, char* challengee);
  void recallChallenge(PlayerData* myData, char* challengee);
  void acceptChallenge(PlayerData* myData, char* challenger);
  void rejectChallenge(PlayerData* myData, char* challenger);

  void makePlayerMove(PlayerData* myData, char* position);
  void showBoard(PlayerData* myData);
  void resignGame(PlayerData* myData);
  void tellPlayer(PlayerData* myData, char* playerName, const char* message);

  // Private helper methods.
  void sendMessageToClient(int sock, const char* formatString, ...);
  void sendGameDataToPlayers(GameData* gameData, bool showPlayerToMove,
                             char* lastMovePosition);

  // Player related methods.
  bool playerExist(char* playerName);
  PlayerData* getPlayerData(char* playerName);
  int getSocket(char* playerName);

  // Challenge related methods.
  bool getChallengeData(int sock, char* challenger,
                        PlayerData*& challengerData, GameData*& gameData);
  bool getGameData(PlayerData* playerData, GameData*& gameData);

  // Data for players and challenges.
  std::map<char*, PlayerData*, StringCompareFunctor> players;
  std::map<char*, GameData*, StringCompareFunctor> challenges;

  int serverPort;
  CommandHandler* commandHandler;
  GameBoardFactory* gameBoardFactory;
};

#endif
