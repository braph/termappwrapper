#ifndef _CMD_WRITE_H
#define _CMD_WRITE_H

#include "iwrap.h"

#include <string.h>
#include <stdio.h>

static
void cmd_write_call(command_call_t *cmd, TermKeyKey *key);

static
command_call_t*
cmd_write_parse(int argc, char *argv[]);

command_t command_write = {
   .name = "write",
   .parse = &cmd_write_parse,
   .call  = &cmd_write_call
};

#endif
