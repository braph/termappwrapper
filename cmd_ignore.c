#include "iwrap.h"

static
void noop(command_call_t *cmd, TermKeyKey *key) {
   (void) 0;
}

command_t command_ignore = {
   .name  = "ignore",
   .desc  = "Do nothing",
   .args  = NULL,
   .opts  = NULL,
   .parse = NULL,
   .call  = &noop,
   .free  = NULL
};
