#include "iwrap.h"
#include "termkeystuff.h"
#include "common.h"
#include "options.h"

#include <string.h>
#include <stdint.h>
#include <unistd.h>

typedef struct cmd_key_args {
   int   repeat;
   char  string;
} cmd_key_args;

static
void key_call(command_call_t *cmd, TermKeyKey *key) {
   cmd_key_args *arg = (cmd_key_args*) cmd->arg;

   for (int i=0; i < arg->repeat; ++i)
      write_to_program(&arg->string);
}

static
char *key_parse_get_code(char *keydef) {
   TermKeyKey key;
   char *seq;

   if (! parse_key(keydef, &key)) {
      write_error("unknown key: %s", keydef);
      return 0;
   }

   if (! (seq = get_key_code(&key))) {
      write_error("could not get key codes for %s", keydef);
      return 0;
   }

   return seq;
}

static
void* key_parse(int argc, char *args[]) {
   char   *seq;
   char   *sequences = NULL;
   int     repeat = 1;
   option *options;

   if (! parse_opts2(&argc, &args, "r:", &options))
      return NULL;

   for (option *opt = options; opt->opt; ++opt) {
      if (opt->opt == 'r')
         if (! (repeat = atoi(opt->arg))) {
            write_error("invalid repeat value: %s", opt->arg);
            free(options);
            return NULL;
         }
   }
   free(options);

   if (! check_args(argc, "+key", 0))
      return NULL;

   if (! (seq = key_parse_get_code(args[0])))
      return NULL;
   sequences = strdup(seq);

   for (int i = 1; i < argc; ++i) {
      if (! (seq = key_parse_get_code(args[i])))
         goto ERROR;

      sequences = realloc(sequences, strlen(sequences) + strlen(seq) + 1);
      strcat(sequences, seq);
   }

   cmd_key_args *cmd_args = malloc(sizeof(*cmd_args) + strlen(sequences));
   cmd_args->repeat = repeat;
   strcpy(&cmd_args->string, sequences);
   return (void*) cmd_args;

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

