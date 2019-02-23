#include "iwrap.h"
#include "common.h"
#include "commands.h"
#include "options.h"
#include "termkeystuff.h"

void *command_create_arg(command_t *cmd, int argc, char **args) {
   option   *options = NULL;
   void     *ret     = NULL;

   if (cmd->opts != NULL) {
      int  i = 0;
      char optstr[64];
      for (const command_opt_t *opt = cmd->opts; opt->opt; ++opt) {
         optstr[i++] = opt->opt;
         if (opt->meta)
            optstr[i++] = ':';
      }
      optstr[i] = 0;

      if (! parse_opts2(&argc, &args, optstr, &options))
         return NULL;
   }

   if (cmd->args == NULL) {
      if (argc > 0) {
         write_error("spare arguments");
         goto ERROR_OR_END;
      }
   }
   else
      if (! check_args_new(argc, cmd->args))
         goto ERROR_OR_END;

   if (cmd->parse != NULL)
      ret = cmd->parse(argc, args, options);
   else
      ret = (void*) 1;

ERROR_OR_END:
   free(options);
   return ret; // is NULL if failed
}

/* parse single command, append to binding */
int binding_append_command(int argc, char *args[], binding_t *binding)
{
   command_t  *cmd = NULL;
   void       *arg = NULL;

   if (! (cmd = get_command(args[0]))) {
      write_error("unknown command: %s", args[0]);
      return 0;
   }

   if (! (arg = command_create_arg(cmd, argc - 1, &args[1]))) {
      prepend_error("%s", cmd->name);
      return 0;
   }

   binding->p.commands = realloc(binding->p.commands, ++binding->size * sizeof(command_call_t));
   binding->p.commands[binding->size - 1].command = cmd;
   binding->p.commands[binding->size - 1].arg = arg;
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
