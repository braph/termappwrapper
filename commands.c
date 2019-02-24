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

void* command_create_arg(command_t *cmd, int argc, char **args) {
   void   *ret     = NULL;
   option *options = NULL;

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


void fprint_command_usage(FILE *fh, command_t *cmd) {
   fprintf(fh, "%-15s%s", cmd->name, cmd->desc);
}

