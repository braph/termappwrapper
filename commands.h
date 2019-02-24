#ifndef _COMMANDS_H
#define _COMMANDS_H

#include "iwrap.h"
#include <stdio.h>

extern command_t* commands[];
extern int        commands_size;

command_t* get_command(char *);
void*      command_create_arg(command_t *, int, char **);
void       fprint_command_usage(FILE *, command_t*);

#endif
