#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

enum Command
{
  // Special command tokens
  UnknownCommand,
  NoDataCommand,
  NameNotGivenYetCommand,

  // Informational commands
  HelpCommand,
  PlayersCommand,
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
  PlayCommand,
  ResignCommand,

  // Other commands
  TellCommand
};

class CommandHandler
{
 public:
  static CommandHandler* getInstance();

  Command parseCommand(char* input, bool nameGiven,
                       char*& restOfLine, char*& dataAfterNewLine);
  char* getFirstWord(char* input, char*& firstWord, char*& restOfString);

 private:
  CommandHandler() {}
  ~CommandHandler() {}

  static CommandHandler* instance;
};

#endif
