#ifndef _CONF_H
#define _CONF_H

#include <stdio.h>
#include "iwrap.h"

typedef struct conf_command_t {
   const char          *name;
   const char          *desc;
   const char         **args;
   const command_opt_t *opts;
   int                (*parse) (int, char **, option*);
} conf_command_t;

extern conf_command_t* conf_commands[];
extern int             conf_commands_size;

conf_command_t* get_conf_command(const char*);
int read_conf_file(const char *);
int read_conf_string(const char *);
int read_conf_stream(FILE *);

#endif
