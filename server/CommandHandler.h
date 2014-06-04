#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <list>
#include <string>

enum Command
{
  // Special command tokens
  UnknownCommand,
  AmbigousCommand,
  NoDataCommand,
  NameNotGivenYetCommand,

  // Informational commands
  HelpCommand,
  WhoCommand,
  GamesCommand,
  ChallengesCommand,

  // Player related commands
  NameCommand,
  GameCommand,

  // Game playing related commands
  ChallengeCommand,
  RecallCommand,
  AcceptCommand,
  RejectCommand,
  MoveCommand,
  BoardCommand,
  ResignCommand,

  // Other commands
  TellCommand
};

class CommandHandler
{
 public:
  static CommandHandler* getInstance();

  const char* getCommandHelp();
  Command parseCommand(char* input, bool nameGiven,
                       char*& restOfLine, char*& dataAfterNewLine);
  char* getFirstWord(char* input, char*& firstWord, char*& restOfString);

 private:
  CommandHandler();

  static CommandHandler* instance;

  void addCommand(const char* commandHelp, Command command,
                  bool needPlayerName);
  struct CommandData
  {
    const char* commandHelp;
    Command command;
    bool needPlayerName;
  };
  std::list<CommandData> commands;
  std::string helpString;
};

#endif
