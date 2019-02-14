#ifndef _CMD_GOTO_H
#define _CMD_GOTO_H

#include "iwrap.h"

void cmd_goto_call(command_call_t *cmd, TermKeyKey *key);

command_call_t*
cmd_goto_parse(int argc, char *args[]);

command_t command_goto = {
   .name = "goto",
   .parse = &cmd_goto_parse,
   .call  = &cmd_goto_call
};

#endif
