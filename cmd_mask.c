#include "iwrap.h"

/*
 * Do not interprete the next key as binding,
 * instead pass the key directly to the program.
 */

static void
mask_call(command_call_t *cmd, TermKeyKey *key) {
   context.mask = 1;
}

command_t command_mask = {
   .name  = "mask",
   .parse = &cmd_parse_none,
   .call  = &mask_call,
   .free  = NULL
};
