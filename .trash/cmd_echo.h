#ifndef _CMD_ECHO_H
#define _CMD_ECHO_H

#include "iwrap.h"
#include <stdio.h>

void cmd_echo_call(command_call_t *cmd, TermKeyKey *key);

command_call_t*
cmd_echo_parse(int argc, char *args[]);

command_t command_echo = {
   .name = "echo",
   .parse = &cmd_echo_parse,
   .call  = &cmd_echo_call
};

#endif
