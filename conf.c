#include "conf.h"
#include "iwrap.h"
#include "common.h"
#include "lexer.h"
#include "termkeystuff.h"

#include <errno.h>
#include <string.h>

// bind_parse.c
int binding_append_commands(int, char *[], binding_t*);

static int lex_args(char ***args) {
   int ttype;
   int n = 0;
   *args = NULL;

   if (lex_eof())
      return EOF;

   while ((ttype = lex()) != EOF) {
      if (ttype == LEX_ERROR)
         return LEX_ERROR;

      if (ttype == LEX_TOKEN_END)
         break;

      *args = realloc(*args, ++n * sizeof(char*));
      (*args)[n - 1] = strdup(lex_token());
   }

   return n;
}

static int mode(int argc, char *args[]) {
   if (! check_args(argc, "mode", 0))
      return 0;

   if (! (context.current_mode = get_keymode(args[0])))
      context.current_mode = add_keymode(args[0]);
   return 1;
}

static int ignore_unmapped(int argc, char *args[]) {
   if (! check_args(argc, "+key types", 0))
      return 0;

   int flags = 0;

   for (int i=0; i < argc; ++i)
      if (strprefix(args[0], "all"))
         flags |= (TERMKEY_TYPE_TO_FLAG(TERMKEY_TYPE_UNICODE) |
                   TERMKEY_TYPE_TO_FLAG(TERMKEY_TYPE_KEYSYM)  |
                   TERMKEY_TYPE_TO_FLAG(TERMKEY_TYPE_FUNCTION));
      else if (strprefix(args[0], "char"))
         flags |= TERMKEY_TYPE_TO_FLAG(TERMKEY_TYPE_UNICODE);
      else if (strprefix(args[0], "sym"))
         flags |= TERMKEY_TYPE_TO_FLAG(TERMKEY_TYPE_KEYSYM);
      else if (strprefix(args[0], "function"))
         flags |= TERMKEY_TYPE_TO_FLAG(TERMKEY_TYPE_FUNCTION);
      else if (strprefix(args[0], "mouse"))
         flags |= TERMKEY_TYPE_TO_FLAG(TERMKEY_TYPE_MOUSE);
      else {
         write_error("unknown key type: %s", args[0]);
         return 0;
      }

   context.current_mode->ignore_unmapped = flags;

   return 1;
}

static int repeat(int argc, char *args[]) {
   if (! check_args(argc, 0))
      return 0;

   context.current_mode->repeat = 1;
   return 1;
}

static int bind(int argc, char *args[]) {
   TermKeyKey key;
   binding_t  *binding;

   if (! check_args(argc, "key", "+command", 0))
      return 0;

   if (! parse_key(args[0], &key))
      return 0;

   binding = keymode_get_binding(context.current_mode, &key);
   if (binding) {
      binding_free(binding);
   }
   else {
      binding = malloc(sizeof(binding_t));
      binding->key        = key;
      binding->commands   = NULL;
      binding->n_commands = 0;
      keymode_add_binding(context.current_mode, binding);
   }

   // TODO FREE ON ERROR
   return binding_append_commands(argc - 1, &args[1], binding);
}

static int unbind(int argc, char *args[]) {
   TermKeyKey key;

   if (! check_args(argc, "key", 0))
      return 0;

   if (! parse_key(args[0], &key))
      return 0;

   keymode_del_binding(context.current_mode, &key);
   return 1;
}

typedef int (*conf_command_t)(int, char*[]);

static struct {
   const char *name;
   int       (*func)(int, char*[]);
} conf_commands[] = {
   { "bind",             &bind            },
   { "unbind",           &unbind          },
   { "mode",             &mode            },
   { "repeat",           &repeat          },
   { "ignore_unmapped",  &ignore_unmapped }
};
#define CONF_COMMANDS_SIZE (sizeof(conf_commands)/sizeof(conf_commands[0]))

static
conf_command_t
get_conf_command(char *name, const char **expaned_name) {
   for (int i=0; i < CONF_COMMANDS_SIZE; ++i)
      if (strprefix(conf_commands[i].name, name)) {
         *expaned_name = conf_commands[i].name;
         return conf_commands[i].func;
      }
   return NULL;
}

int read_conf_stream(FILE *fh) {
   int         ret = 0;
   char        **args = NULL;
   int         nargs;

   const char *expaned_name;
   conf_command_t conf_command;

   lex_init(fh);

   while ((nargs = lex_args(&args)) != EOF) {
      if (nargs == LEX_ERROR) {
         write_error("%s", lex_error());
         ret = -1;
         goto END;
      }

      if (nargs == 0) {
         continue;
      }

      if (! (conf_command = get_conf_command(args[0], &expaned_name))) {
         write_error("unknown command: %s", args[0]);
         ret = -1;
         goto END;
      }
      else if (! conf_command(nargs - 1, &args[1])) {
         prepend_error("%s", expaned_name);
         ret = -1;
         goto END;
      }

      freeArray(args, nargs);
      args = NULL;
   }

END:
   if (args)
      freeArray(args, nargs);
   lex_destroy();
   return ret;
}

int read_conf_string(const char *str) {
   FILE *fh = fmemopen((void*) str, strlen(str), "r");

   if (! fh) {
      write_error("%s", strerror(errno));
      return -1;
   }

   int ret = read_conf_stream(fh);
   fclose(fh);
   return ret;
}

int read_conf_file(const char *file) {
   FILE *fh = fopen(file, "r");

   if (! fh) {
      write_error("%s", strerror(errno));
      return -1;
   }

   int ret = read_conf_stream(fh);
   fclose(fh);
   return ret;
}
