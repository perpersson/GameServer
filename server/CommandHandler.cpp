#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "CommandHandler.h"

using namespace std;

CommandHandler* CommandHandler::instance = NULL;

CommandHandler* CommandHandler::getInstance()
{
  if (instance == NULL)
    instance = new CommandHandler();
  return instance;
}

CommandHandler::CommandHandler()
{
  addCommand("help                  Show this help", HelpCommand, false);
  addCommand("name <your name>      Set your name", NameCommand, false);
  addCommand("game <game to play>   Set the game you want to play", GameCommand, true);
  addCommand("who                   List all players", WhoCommand, false);
  addCommand("games                 List all games that are possible to play", GamesCommand, false);
  addCommand("challenges            List challenges to/from you", ChallengesCommand, true);
  addCommand("challenge <player>    Challenge another player", ChallengeCommand, true);
  addCommand("recall <player>       Recall your challenge", RecallCommand, true);
  addCommand("accept <player>       Accept challenge from another player", AcceptCommand, true);
  addCommand("reject <player>       Reject challenge from another player", RejectCommand, true);
  addCommand("move <position>       Make a move in the ongoing game", MoveCommand, true);
  addCommand("board                 Show board for ongoing game", BoardCommand, true);
  addCommand("resign                Give up current game", ResignCommand, true);
  addCommand("tell <player> <msg>   Send a message to player", TellCommand, true);
  addCommand("quit                  Disconnect from server", QuitCommand, false);

  // Add final help text.
  helpString += "\nCommand abbreviations are allowed when not ambigous\n";
}

void CommandHandler::addCommand(const char* commandHelp, Command command,
                                bool needPlayerName)
{
  CommandData data;
  data.commandHelp = commandHelp;
  data.command = command;
  data.needPlayerName = needPlayerName;
  commands.push_back(data);

  // Add to command help string.
  helpString += commandHelp;
  helpString += '\n';
}

const char* CommandHandler::getCommandHelp()
{
  return helpString.c_str();
}

Command CommandHandler::parseCommand(char* input, bool nameGiven,
                                     char*& restOfLine,
                                     char*& dataAfterNewLine)
{
  // Check for new line characters in input.
  dataAfterNewLine = strchr(input, '\n');
  if (dataAfterNewLine != NULL)
    *dataAfterNewLine++ = '\0';

  // Remove trailing spaces in command.
  char* p = strchr(input, '\0');
  while (--p > input && isspace(*p))
    *p = '\0';

  // Get first word from input and check if it matches any command.
  char* firstWord;
  getFirstWord(input, firstWord, restOfLine);
  unsigned int firstWordLength = strlen(firstWord);
  if (firstWordLength == 0)
    return NoDataCommand;

  // Check if there's a match for any command. Full or shortened match.
  Command bestMatchingCommand = UnknownCommand;
  for (auto iterator=commands.begin(); iterator!=commands.end(); ++iterator)
  {
    // Matches beginning of a command?
    if (strncmp(firstWord, iterator->commandHelp, firstWordLength) == 0)
    {
      if (isspace(iterator->commandHelp[firstWordLength]))
      {
        // Exact match of command.
        if (nameGiven || !iterator->needPlayerName)
          return iterator->command;
        else
          return NameNotGivenYetCommand;
      }
      else
      {
        // Match of beginning of command.
        if (bestMatchingCommand != UnknownCommand)
          bestMatchingCommand = AmbigousCommand;
        else
          bestMatchingCommand = iterator->command;
      }
    }
  }
  return bestMatchingCommand;
}

char* CommandHandler::getFirstWord(char* input,
                                   char*& firstWord, char*& restOfString)
{
  // Search for start of first word in input.
  while (isspace(*input))
    ++input;
  firstWord = input;

  // End of string without any data?
  if (*input == '\0')
    return restOfString = firstWord;

  // Get the entire word.
  while (!isspace(*input) && *input != '\0')
    ++input;

  restOfString = input;
  if (*input != '\0')
  {
    *input = '\0';   // Terminate firstWord.
    restOfString = input + 1;
  }

  return firstWord;
}
