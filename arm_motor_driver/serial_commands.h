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

typedef void (*Command) (void **args);

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
    Command commands[MAX_COMMANDS];
    CommandArgType command_arguments[MAX_COMMANDS][MAX_COMMAND_ARGUMENTS];

    // commands ids are single characters
    int addCommand(char command_id, Command command, CommandArgType types[MAX_COMMAND_ARGUMENTS]) 
    {
      command_ids[command_count] = command_id;
      commands[command_count] = command;
      
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
      char *command_ptr = command_str+1;
      void **argument_stack = (void**) malloc(sizeof(void*)*4);
      int given_args = 0;
      for (int i = 0; i < MAX_COMMAND_ARGUMENTS; i++)
      {
        const CommandArgType arg_type = command_arguments[command_index][i];
        char *end_ptr;
        void *arg;

        switch (arg_type) 
        {
          case INT:
            arg = new int((int) strtol(command_ptr, &end_ptr, 10));
            break;
          case FLOAT:
            arg = new float(strtof(command_ptr, &end_ptr));
            break;
          case NONE:
            break;
        }

        // Either invalid argument or no arguments left 
        if (end_ptr == command_ptr)
          break;

        given_args++;
        argument_stack[i] = arg;
      }

      commands[command_index](argument_stack);
      
      // Freeing the stack
      for (int i = 0; i < given_args; i++)
      {
        const CommandArgType arg_type = command_arguments[command_index][i];
        switch (arg_type) 
        {
          case INT:
          delete (int*) argument_stack[i];
            break;
          case FLOAT:
            delete (float*) argument_stack[i];
            break;
          case NONE:
            break;
        }
      }

      free(argument_stack);

      return 1;
    }
};
#endif