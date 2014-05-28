#ifndef GAME_SERVER
#define GAME_SERVER

#include <map>
#include <string.h>
#include "PlayerData.h"
#include "GameBoardFactory.h"
#include "StringCompareFunctor.h"

class GameServer
{
 public:
  GameServer();

  void mainLoop();

 private:
  static void* handleClient(void* sockArg);

  // Client command methods.
  static void showCommands(int sock);
  static void listPlayers(int sock);
  static void listGames(int sock);
  static void listChallenges(PlayerData* myData);

  static void addPlayer(char* playerName, int sock, PlayerData*& playerData);
  static void setFavouriteGame(PlayerData* myData, char* game);

  static void challengeOtherPlayer(PlayerData* myData, char* challengee);
  static void recallChallenge(PlayerData* myData, char* challengee);
  static void acceptChallenge(PlayerData* myData, char* challenger);
  static void rejectChallenge(PlayerData* myData, char* challenger);

  static void makePlayerMove(PlayerData* myData, char* position);
  static void resignGame(PlayerData* myData);
  static void tellPlayer(PlayerData* myData, char* playerName,
                         const char* message);

  // Private helper methods.
  static void sendMessageToClient(int sock, const char* formatString, ...);
  static void sendGameBoardToPlayers(GameData* gameData);

  // Player related methods.
  static bool playerExist(char* playerName);
  static PlayerData* getPlayerData(char* playerName);
  static int getSocket(char* playerName);

  // Challenge related methods.
  static bool getChallengeData(int sock, char* challenger,
			       PlayerData*& challengerData,
			       GameData*& gameData);

  // Data for players and challenges.
  static std::map<char*, PlayerData*, StringCompareFunctor> players;
  static std::map<char*, GameData*, StringCompareFunctor> challenges;  
};

#endif
