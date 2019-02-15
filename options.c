#include "options.h"

#include "iwrap.h"
#include <string.h>
#include <stdlib.h>

/*
 * Parse options, return the index of first non option or -1 on failure
 */
int parse_opts(int argc, char *args[], const char *optstr, option **opts) {
   int    i;
   char  *c;
   char  *oc;
   int    opti = -1;
   *opts = NULL;

   for (i = 0; i < argc; ++i) {
      if (args[i][0] != '-')   // is argument
         goto RETURN;
      if (args[i][1] == 0)     // argument "-"
         goto RETURN;
      if (args[i][1] == '-') { // end of options "--"
         if (args[i][2] != '-') {
            write_error("long options not supported: %s", args[i]);
            goto ERROR;
         }
         else
            goto RETURN;
      }

      c = &args[i][1];
      do {
         if (! (oc = strchr(optstr, *c))) {
            write_error("unknown option: %c\n", *c);
            goto ERROR;
         }

         *opts = realloc(*opts, (1 + ++opti) * sizeof(option));
         (*opts)[opti].opt = *c;

         if (*(oc+1) == ':') {  // needs argument
            if (*(c+1) != 0) {  // arg is in "-oARG"
               (*opts)[opti].arg = (c+1);
               break;
            }
            if (++i < argc) {   // arg is "-o ARG"
               (*opts)[opti].arg = args[i];
               break;
            }
            else {
               write_error("missing argument: %c\n", *c);
               goto ERROR;
            }
         }
      }
      while (*++c);
   }

RETURN:
   *opts = realloc(*opts, (1 + ++opti) * sizeof(option));
   (*opts)[opti].opt = 0;
   return i;

ERROR:
   free(opts);
   return -1;
}

/*
 * Parse options, modify argc and argv, return 1 on success, 0 on failure
 */
int parse_opts2(int *argc, char **args[], const char *optstr, option **opts) {
   int optind = parse_opts(*argc, *args, optstr, opts);
   if (optind == -1)
      return 0;

   *argc -= optind;
   *args  = &(*args)[optind];
   return 1;
}
