#ifndef _OPTIONS_H
#define _OPTIONS_H

typedef struct option {
   char        opt;
   const char *arg;
} option;

int parse_options(int argc, char *args[], const char *optstr, option **opts);
int get_options(int *argc, char **args[], const char *optstr, option **opts);

#endif
