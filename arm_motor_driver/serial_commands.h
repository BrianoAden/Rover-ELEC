#ifndef SerialCommands
#define SerialCommands

#include <stdlib.h>

#if VERBOSE_COMMANDS
#ifndef COMMAND_SERIAL
  #error "COMMAND_SERIAL must be defined to use VERBOSE_COMMANDS in serial_commands.h"
#endif
#define sc_print(...) COMMAND_SERIAL.print(__VA_ARGS__)
#define sc_println(...) COMMAND_SERIAL.println(__VA_ARGS__)
#else
#define sc_print(...) 
#define sc_println(...) 
#endif

#define MAX_COMMAND_LENGTH 50
#define MAX_COMMANDS 10
#define MAX_COMMAND_ARGUMENTS 3

#define ARGUMENT_SPACER ' '

typedef void (*Command) (void* arg1, void* arg2, void * arg3);

enum CommandArgType {
  NONE = 0, // unsigned types are none
  INT,
  FLOAT,
  CHAR
};

class CommandHandler
{
  public:
    int command_count = 0;
    char command_ids[MAX_COMMANDS];
    CommandArgType command_arguments[MAX_COMMANDS][MAX_COMMAND_ARGUMENTS];

    // commands ids are single characters
    int addCommand(char command_id, Command command, CommandArgType types[MAX_COMMAND_ARGUMENTS]) 
    {
      command_ids[command_count] = command_id;
      
      // Copying argument types
      for (char i = 0; i < MAX_COMMAND_ARGUMENTS; i++)
        command_arguments[command_count][i] = types[i];

      command_count++;
      sc_print(F("Added command: "));
      sc_println(message);
    }

    int runCommand(char* command_str)
    {
      char command_id = command_str[0];

      int command_index = -1;
      for (int i = 0; i < command_count; i++)
      {
        if (command_id == command_ids[i])
        {
          command_index = i;
          break;
        }
      } 

      // Checking if a command was found
      if (command_index == -1)
      {
        sc_print(F("ERROR: Invalud command id: \""));
        sc_print(command_id);
        sc_println(F("\""));
        return 0;
      }

      // Gathering arguments
      
      for (int i = 0; i < MAX_COMMAND_ARGUMENTS; i++)
      {
        const CommandArgType arg_type = command_arguments[command_index][i];
        switch (arg_type) 
        {
          case INT:
            int arg = 
        }
      }

        
      return 1;
    }
};
#endif