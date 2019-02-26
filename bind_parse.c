#include "iwrap.h"
#include "common.h"
#include "commands.h"
#include "options.h"
#include "termkeystuff.h"

/* parse single command, append to binding */
static
int binding_append_command(binding_t *binding, int argc, char *args[])
{
   command_t *cmd = NULL;
   void      *arg = NULL;

   if (! (cmd = get_command(args[0]))) {
      write_error("unknown command: %s", args[0]);
      return 0;
   }

   if (! (arg = command_parse(cmd, argc - 1, &args[1]))) {
      prepend_error("%s", cmd->name);
      return 0;
   }

   binding->p.commands = realloc(binding->p.commands, ++binding->size * sizeof(command_call_t));
   binding->p.commands[binding->size - 1].command = cmd;
   binding->p.commands[binding->size - 1].arg = arg;
   return 1;
}

/* parse multiple commands, append to binding */
int binding_append_commands(binding_t *binding, int argc, char *args[])
{
   int j;

   for (int i = 0; i < argc; ++i) {
      for (j = i + 1; j < argc; ++j)
         if (streq(args[j], "\\;"))
            break;

      if (! binding_append_command(binding, j - i, &args[i]))
         return 0;
      i = j;
   }

   return 1;
}
