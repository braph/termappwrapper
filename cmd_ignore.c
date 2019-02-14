#include "termkeystuff.h"
#include "iwrap.h"

/*
 * Do nothing
 */

static
void ignore(command_call_t *cmd, TermKeyKey *key) {
   (void)0;
}

command_t command_ignore = {
   .name  = "ignore",
   .parse = &cmd_parse_none,
   .call  = &ignore,
   .free  = NULL
};
