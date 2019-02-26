#ifndef _COMMANDS_H
#define _COMMANDS_H

#include "iwrap.h"
#include <stdio.h>

extern command_t* commands[];
extern int        commands_size;

command_t* get_command(const char *);
void*      command_parse(command_t *, int, char **);

#endif
