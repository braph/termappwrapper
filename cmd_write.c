#include "iwrap.h"

#include <string.h>
#include <unistd.h>

static
void write_call(command_call_t *cmd, TermKeyKey *key) {
   write_to_program((char*) cmd->arg);
}

static
void* write_parse(int argc, char **args) {
   if (! check_args(argc, "+string", 0))
      return NULL;

   char *string = strdup(args[0]);

   for (int i = 1; i < argc; ++i) {
      string = realloc(string, strlen(string) + strlen(args[i]) + 1);
      strcat(string, args[i]);
   }

   return (void*) string;
}

command_t command_write = {
   .name  = "write",
   .parse = &write_parse,
   .call  = &write_call,
   .free  = &free
};

