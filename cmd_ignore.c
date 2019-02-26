#include "iwrap.h"

static COMMAND_CALL_FUNC(call) {
   (void) 0;
}

const command_t command_ignore = {
   .name  = "ignore",
   .desc  = "Do nothing\n"
            "This command can be used to ignore keypresses.",
   .args  = NULL,
   .opts  = NULL,
   .parse = NULL,
   .call  = &call,
   .free  = NULL
};
