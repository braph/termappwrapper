#include "iwrap.h"
#include "termkeystuff.h"

#include <string.h>
#include <stdint.h>
#include <unistd.h>

typedef struct cmd_key_args {
   short repeat;
   char  string[1];
} cmd_key_args;

static
void key_call(command_call_t *cmd, TermKeyKey *key) {
   cmd_key_args *arg = (cmd_key_args*) cmd->arg;

   for (int i = arg->repeat; i--; )
      write_to_program(arg->string);
}

static
char *key_parse_get_code(char *keydef) {
   TermKeyKey key;
   char *seq;

   if (! parse_key(keydef, &key))
      return 0;

   if (! (seq = get_key_code(&key)))
      write_error("Could not get key code for %s", keydef);

   return seq; // is NULL if failed
}

static
void* key_parse(int argc, char *args[], option *options) {
   char         *seq;
   char         *sequences = NULL;
   int           repeat = 1;
   cmd_key_args *cmd_args = NULL;

   for (option *opt = options; opt->opt; ++opt) {
      if (opt->opt == 'r')
         if ((repeat = atoi(opt->arg)) <= 0) {
            write_error("invalid repeat value: %s", opt->arg);
            return NULL;
         }
   }

   if (! (seq = key_parse_get_code(args[0])))
      return NULL;
   sequences = strdup(seq);

   for (int i = 1; i < argc; ++i) {
      if (! (seq = key_parse_get_code(args[i])))
         goto END_OR_ERROR;

      sequences = realloc(sequences, strlen(sequences) + strlen(seq) + 1);
      strcat(sequences, seq);
   }

   cmd_args = malloc(sizeof(*cmd_args) + strlen(sequences));
   cmd_args->repeat = repeat;
   strcpy(cmd_args->string, sequences);

END_OR_ERROR:
   free(sequences);
   return (void*) cmd_args; // is NULL if failed
}

command_t command_key = {
   .name  = "key",
   .desc  = "Send key",
   .args  = (const char*[]) { "+KEY", 0 },
   .opts  = (const command_opt_t[]) {{'r', "NUM", "Repeat key"}, {0,0,0}},
   .parse = &key_parse,
   .call  = &key_call,
   .free  = &free
};

