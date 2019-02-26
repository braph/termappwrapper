#include "iwrap.h"
#include "options.h"
#include "error_messages.h"

#include <string.h>
#include <stdlib.h>

/*
 * Parse options, return the index of first non option or -1 on failure
 */
int parse_options(int argc, char *args[], const char *optstr, option **opts) {
   int i;
   int opti = -1;
   const char *c;
   const char *oc;
   *opts = NULL;

   for (i = 0; i < argc; ++i) {
      if (args[i][0] != '-')    // is argument
         goto RETURN;
      if (args[i][1] == 0)      // argument "-"
         goto RETURN;
      if (args[i][1] == '-') { 
         if (args[i][2] == '0') // end of options "--"
            goto RETURN;
         else {
            // treat --long-options as unknown option
            write_error("%s: %s", E_UNKNOWN_OPT, args[i]);
            goto ERROR;
         }
      }

      c = &args[i][1];
      do {
         if (! (oc = strchr(optstr, *c))) {
            write_error("%s: %c\n", E_UNKNOWN_OPT, *c);
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
               write_error("%s: %c\n", E_MISSING_ARG, *c);
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
int get_options(int *argc, char **args[], const char *optstr, option **opts) {
   int optind = parse_options(*argc, *args, optstr, opts);
   if (optind == -1)
      return 0;

   *argc -= optind;
   *args  = &(*args)[optind];
   return 1;
}
