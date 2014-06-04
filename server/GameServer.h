#ifndef GAME_SERVER_H
#define GAME_SERVER_H

#include <map>
#include <mutex>
#include <string.h>
#include "CommandHandler.h"
#include "GameBoardFactory.h"
#include "PlayerData.h"
#include "StringCompareFunctor.h"

class GameServer
{
 public:
  GameServer();

  void clientThreadMainLoop(int sock);

 private:
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
                             char* previousMovePosition);

  // Player related methods.
  bool playerExist(char* playerName);
  PlayerData* getPlayerData(char* playerName);
  int getSocket(char* playerName);

  // Challenge related methods.
  bool getChallengeData(int sock, char* challenger,
                        PlayerData*& challengerData, GameData*& gameData);
  bool getGameData(PlayerData* playerData, GameData*& gameData);

  // Data for players and challenges.
  std::mutex commandMutex;
  std::map<char*, PlayerData*, StringCompareFunctor> players;
  std::map<char*, GameData*, StringCompareFunctor> challenges;

  CommandHandler* commandHandler;
  GameBoardFactory* gameBoardFactory;
};

#endif
