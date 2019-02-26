#include "iwrap.h"

static COMMAND_CALL_FUNC(call) {
   context.mask = 1;
}

const command_t command_mask = {
   .name  = "mask",
   .desc  = "Do not interprete the next keypress",
   .args  = NULL,
   .opts  = NULL,
   .parse = NULL,
   .call  = &call,
   .free  = NULL
};
