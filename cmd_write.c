#include "iwrap.h"

#include <string.h>
#include <unistd.h>

typedef struct cmd_write_args {
   short repeat;
   char  string[1];
} cmd_write_args;

static COMMAND_CALL_FUNC(call) {
   cmd_write_args *arg = (cmd_write_args*) cmd->arg;

   for (int i = arg->repeat; i--; )
      writes_to_program(arg->string);
}

static COMMAND_PARSE_FUNC(parse) {
   int i;
   int size = 1;
   int repeat = 1;

   for (option *opt = options; opt->opt; ++opt) {
      if (opt->opt == 'r')
         if ((repeat = atoi(opt->arg)) <= 0) {
            write_error("invalid repeat value: %s", opt->arg);
            return NULL;
         }
   }

   for (i = argc; i--;)
      size += strlen(args[i]);

   cmd_write_args *cmd_args = malloc(sizeof(*cmd_args) + size);
   cmd_args->repeat = repeat;

   strcpy(cmd_args->string, args[0]);
   for (i = 1; i < argc; ++i)
      strcat(cmd_args->string, args[i]);

   return (void*) cmd_args;
}

command_t command_write = {
   .name  = "write",
   .desc  = "Write string to program",
   .args  = (const char*[]) { "+STRING", 0 },
   .opts  = (const command_opt_t[]) {{ 'r', "NUM", "repeat" }, {0,0,0}},
   .parse = &parse,
   .call  = &call,
   .free  = &free
};
