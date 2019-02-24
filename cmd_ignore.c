#include "iwrap.h"

static COMMAND_CALL_FUNC(call) {
   (void) 0;
}

command_t command_ignore = {
   .name  = "ignore",
   .desc  = "Do nothing",
   .args  = NULL,
   .opts  = NULL,
   .parse = NULL,
   .call  = &call,
   .free  = NULL
};
