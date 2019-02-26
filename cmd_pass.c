#include "iwrap.h"

static COMMAND_CALL_FUNC(call) {
   writeb_to_program(context.input_buffer, context.input_len);
}

const command_t command_pass = {
   .name  = "pass",
   .desc  = "Pass the current key to the program",
   .args  = NULL,
   .opts  = NULL,
   .parse = NULL,
   .call  = &call,
   .free  = NULL
};
