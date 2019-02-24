#include "iwrap.h"

static void
call(command_call_t *cmd, TermKeyKey *key) {
   context.current_mode = (keymode_t*) cmd->arg;
}

static void*
parse(int argc, char *args[], option* options) {
   keymode_t *km = get_keymode(args[0]);

   if (! km)
      return add_keymode(args[0]);

   return (void*) km;
}

command_t command_goto = {
   .name  = "goto",
   .desc  = "Switch current input mode",
   .args  = (const char*[]) { "MODE", 0 },
   .opts  = NULL,
   .call  = &call,
   .parse = &parse,
   .free  = NULL
};
