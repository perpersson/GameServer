#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "CommandHandler.h"

CommandHandler* CommandHandler::instance = NULL;

CommandHandler* CommandHandler::getInstance()
{
  if (instance == NULL)
    instance = new CommandHandler();
  return instance;
}

Command CommandHandler::parseCommand(char* input, bool nameGiven,
                                     char*& restOfLine,
                                     char*& dataAfterNewLine)
{
  struct CommandData
  {
    char* commandName;
    bool needPlayerName;
  };
  const CommandData commandData[] =
    {{"unknown", false},
     {"nodata", false},
     {"namenotgivenyet", false},
     {"help", false},
     {"players", false},
     {"games", false},
     {"challenges", true},
     {"name", false},
     {"game", true},
     {"challenge", true},
     {"recall", true},
     {"accept", true},
     {"reject", true},
     {"play", true},
     {"resign", true},
     {"tell", true}};

  // Check for new line characters in input.
  dataAfterNewLine = strchr(input, '\n');
  if (dataAfterNewLine != NULL)
    *dataAfterNewLine++ = '\0';

  // Get first word from input and check if it matches any command.
  char* firstWord;
  getFirstWord(input, firstWord, restOfLine);
  for (int command=HelpCommand; command<=TellCommand; ++command)
  {
    if (strcmp(firstWord, commandData[command].commandName) == 0)
    {
      if (nameGiven || !commandData[command].needPlayerName)
        return (Command)command;
      else
        return NameNotGivenYetCommand;
    }
  }
  return (strlen(firstWord) == 0 ? NoDataCommand : UnknownCommand);
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
