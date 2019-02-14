#ifndef _CMD_SIGNAL_H
#define _CMD_SIGNAL_H

#include "iwrap.h"

void cmd_signal_call(command_call_t *cmd, TermKeyKey *key);

command_call_t*
cmd_signal_parse(int argc, char *args[]);

command_t command_signal = {
   .name = "signal",
   .parse = &cmd_signal_parse,
   .call  = &cmd_signal_call
};

#endif
