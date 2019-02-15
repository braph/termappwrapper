#ifndef _OPTIONS_H
#define _OPTIONS_H

typedef struct option {
   char  opt;
   char *arg;
} option;

int parse_opts(int argc, char *args[], const char *optstr, option **opts);
int parse_opts2(int *argc, char **args[], const char *optstr, option **opts);

#endif
