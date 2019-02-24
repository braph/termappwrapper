#ifndef _COMMANDS_H
#define _COMMANDS_H

#include "iwrap.h"
#include <stdio.h>

//#define COMMANDS_SIZE (sizeof(commands)/sizeof(commands[0]))
#define COMMANDS_SIZE 7

extern command_t* commands[];

command_t* get_command(char *);
void*      command_create_arg(command_t *cmd, int argc, char **args);

void       fprint_command_usage(FILE *, command_t*);

#endif
