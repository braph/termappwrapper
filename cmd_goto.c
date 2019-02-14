#include "iwrap.h"

/*
 * Switch mode
 */

static void
goto_call(command_call_t *cmd, TermKeyKey *key) {
   context.current_mode = (keymode_t*) cmd->arg;
}

static void*
goto_parse(int argc, char *args[]) {
   if (! check_args(argc, "section", 0))
      return NULL;

   keymode_t *km = get_keymode(args[0]);
   if (! km) {
      write_error("section not found: %s", args[0]);
      return NULL;
   }

   return (void*) km;
}

command_t command_goto = {
   .name  = "goto",
   .call  = &goto_call,
   .parse = &goto_parse,
   .free  = NULL
};
