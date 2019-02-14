#include "cmd_echo.h"
#include "termkeystuff.h"

void cmd_echo_call(command_call_t *cmd, TermKeyKey *key) {
   printf("%s\n", format_key(key));
}

command_call_t*
cmd_echo_parse(int argc, char *args[]) {
   if (argc) {
      write_error("spare arguments");
      return NULL;
   }

   return command_call_new(&cmd_echo_call, NULL);
}

