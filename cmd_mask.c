#include "iwrap.h"

static void
mask_call(command_call_t *cmd, TermKeyKey *key) {
   context.mask = 1;
}

command_t command_mask = {
   .name  = "mask",
   .desc  = "Mask the next input character (do not interprete it as a binding)",
   .args  = NULL,
   .opts  = NULL,
   .parse = NULL,
   .call  = &mask_call,
   .free  = NULL
};
