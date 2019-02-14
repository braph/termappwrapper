#include "iwrap.h"

static void
cmd_repeat_call(command_call_t *cmd, TermKeyKey *key) {
   int n = key->code.codepoint - 0x30;
   context.repeat = context.repeat * 10 + n;
}

static
command_call_t*
cmd_repeat_parse(int argc, char *args[]) {
   if (argc > 0) {
      write_error("spare arguments");
      return NULL;
   }

   return command_call_new(&cmd_repeat_call, NULL);
}

command_t command_repeat = {
   .name = "repeat",
   .parse = &cmd_repeat_parse,
   .call  = &cmd_repeat_call
};
