#include "iwrap.h"
#include "common.h"
#include "termkeystuff.h"

extern command_t command_key;
extern command_t command_goto;
extern command_t command_mask;
extern command_t command_write;
extern command_t command_signal;
extern command_t command_ignore;

static command_t* commands[] = {
   &command_key,
   &command_goto,
   &command_mask,
   &command_write,
   &command_signal,
   &command_ignore
};
#define COMMANDS_SIZE (sizeof(commands)/sizeof(commands[0]))

static
command_t* get_command(char *name) {
   for (int i=0; i < COMMANDS_SIZE; ++i)
      if (strprefix(commands[i]->name, name))
         return commands[i];

   return NULL;
}

/* parse single command, appends to binding */
int binding_append_command(int argc, char *args[], binding_t *binding)
{
   command_t   *cmd = NULL;
   void        *arg = NULL;

   if (! (cmd = get_command(args[0]))) {
      write_error("unknown command: %s", args[0]);
      return 0;
   }

   if (! (arg = cmd->parse(argc - 1, &args[1]))) {
      prepend_error("%s", cmd->name);
      return 0;
   }

   binding->commands = realloc(binding->commands, ++binding->n_commands * sizeof(command_call_t));
   binding->commands[binding->n_commands - 1].command = cmd;
   binding->commands[binding->n_commands - 1].arg = arg;
   return 1;
}

/* parse multiple commands, append to binding */
int binding_append_commands(int argc, char *args[], binding_t *binding)
{
   int j;

   for (int i = 0; i < argc; ++i) {
      for (j = i + 1; j < argc; ++j)
         if (streq(args[j], "\\;"))
            break;

      if (! binding_append_command(j - i, &args[i], binding))
         return 0;
      i = j;
   }

   return 1;
}
