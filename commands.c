#include "commands.h"
#include "common.h"

extern command_t command_key;
extern command_t command_goto;
extern command_t command_mask;
extern command_t command_write;
extern command_t command_signal;
extern command_t command_ignore;
extern command_t command_readline;

command_t* commands[] = {
   &command_key,
   &command_goto,
   &command_mask,
   &command_write,
   &command_signal,
   &command_ignore,
   &command_readline
};

command_t* get_command(char *name) {
   for (int i = COMMANDS_SIZE; i--;)
      if (strprefix(commands[i]->name, name))
         return commands[i];

   return NULL;
}

void fprint_command_usage(FILE *fh, command_t *cmd) {
   fprintf(fh, "%-15s%s", cmd->name, cmd->desc);
}

