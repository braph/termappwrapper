#include "iwrap.h"
#include "termkeystuff.h"
#include "common.h"

#include <string.h>
#include <unistd.h>

static
void key_call(command_call_t *cmd, TermKeyKey *key) {
   write_to_program((char*) cmd->arg);
}

static
void* key_parse(int argc, char *args[]) {
   TermKeyKey key;
   char  *seq;
   char  *sequences = NULL;

   if (! check_args(argc, "+key", 0))
      return NULL;

   for (int i=0; i < argc; ++i) {
      if (! parse_key(args[i], &key)) {
         write_error("unknown key: %s", args[i]);
         goto ERROR;
      }

      if (! (seq = get_key_code(&key))) {
         write_error("could not get key codes for %s", args[i]);
         goto ERROR;
      }

      if (sequences == NULL) {
         sequences = strdup(seq);
      }
      else {
         sequences = realloc(sequences, strlen(sequences) + strlen(seq) + 1);
         strcat(sequences, seq);
      }
   }

   return (void*) sequences;

ERROR:
   free(sequences);
   return NULL;
}

command_t command_key = {
   .name  = "key",
   .parse = &key_parse,
   .call  = &key_call,
   .free  = &free
};

