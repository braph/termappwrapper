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

binding_t* bind_parse(int argc, char *args[])
{
   TermKeyKey     *key      = NULL;
   command_t      *cmd      = NULL;
   binding_t      *binding  = NULL;

   if (! check_args(argc, "key", "+command"))
      return NULL;

   if (! (key = parse_key_new(args[0]))) {
      write_error("invalid key %s", args[0]);
      return NULL;
   }

   binding = malloc(sizeof(binding_t));
   binding->key        = key;
   binding->commands   = NULL;
   binding->n_commands = 0;

   int j, sub_argc;
   for (int i = 1; i < argc; ++i) {
      if (! (cmd = get_command(args[i]))) {
         write_error("unknown command: %s", args[i]);
         goto ERROR;
      }

      for (j = i + 1; j < argc; ++j) {
         if (streq(args[j], ";"))
            break;
      }
      sub_argc = j - i - 1;

      binding->commands = realloc(binding->commands, ++binding->n_commands * sizeof(command_call_t));
      binding->commands[binding->n_commands - 1].command = cmd;

      void *cmdarg = cmd->parse(sub_argc, &args[i + 1]);
      if (! cmdarg) {
         prepend_error("%s", cmd->name);
         goto ERROR;
      }

      binding->commands[binding->n_commands - 1].arg = cmdarg;

      i = j;
   }

   return binding;

ERROR:
   free(key);
   free(binding);
   //freeArray(commands, n_commands);
   return NULL;
}
