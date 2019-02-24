#include "conf.h"
#include "iwrap.h"
#include "common.h"
#include "lexer.h"
#include "termkeystuff.h"

#include <errno.h>
#include <string.h>

// bind_parse.c
int binding_append_commands(binding_t*, int, char *[]);

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

   for (int i = argc; i--; )
      if (strprefix(args[i], "all"))
         flags |= (TERMKEY_TYPE_TO_FLAG(TERMKEY_TYPE_UNICODE) |
                   TERMKEY_TYPE_TO_FLAG(TERMKEY_TYPE_KEYSYM)  |
                   TERMKEY_TYPE_TO_FLAG(TERMKEY_TYPE_FUNCTION));
      else if (strprefix(args[i], "char"))
         flags |= TERMKEY_TYPE_TO_FLAG(TERMKEY_TYPE_UNICODE);
      else if (strprefix(args[i], "sym"))
         flags |= TERMKEY_TYPE_TO_FLAG(TERMKEY_TYPE_KEYSYM);
      else if (strprefix(args[i], "function"))
         flags |= TERMKEY_TYPE_TO_FLAG(TERMKEY_TYPE_FUNCTION);
      else if (strprefix(args[i], "mouse"))
         flags |= TERMKEY_TYPE_TO_FLAG(TERMKEY_TYPE_MOUSE);
      else {
         write_error("unknown key type: %s", args[i]);
         return 0;
      }

   context.current_mode->ignore_unmapped = flags;
   return 1;
}

static int repeat(int argc, char *args[]) {
   if (! check_args_new(argc, (const char*[]) { "on|off", 0 }))
      return 0;

   if (streq(args[0], "on"))
      context.current_mode->repeat_enabled = 1;
   else if (streq(args[0], "off"))
      context.current_mode->repeat_enabled = 0;
   else {
      write_error("argument has to be 'on' or 'off'");
      return 0;
   }

   return 1;
}

static int bind(int argc, char *args[]) {
   TermKeyKey key;
   binding_t  *binding, *binding_next;

   binding = context.current_mode->root;

   // read till last keydef
   while (argc > 1 && parse_key(args[0], &key)) {
      args_get_arg(&argc, &args, NULL);
      binding_next = binding_get_binding(binding, &key);

      if (! binding_next) {
         binding_next             = malloc(sizeof(binding_t));
         binding_next->key        = key;
         binding_next->size       = 0;
         binding_next->p.commands = NULL;
         binding_next->type       = BINDING_TYPE_CHAINED;
         binding = binding_add_binding(binding, binding_next);
      }
      else {
         if (binding_next->type == BINDING_TYPE_COMMAND) {
            write_error("Overwriting key binding"); // TODO
            return 0;
         }

         binding = binding_next;
      }
   }

   if (binding == context.current_mode->root) {
      write_error("missing key");
      return 0;
   }

   if (binding->type == BINDING_TYPE_COMMAND) {
      write_error("Overwriting key binding");
      return 0;
   }

   if (binding->type == BINDING_TYPE_CHAINED && binding->size > 0) {
      write_error("Overwriting key binding");
      return 0;
   }

   if (argc == 0) {
      write_error("missing command");
      return 0;
   }

   binding->type = BINDING_TYPE_COMMAND;
   return binding_append_commands(binding, argc, args);
}

static int unbind(int argc, char *args[]) {
   TermKeyKey key;

   if (! check_args(argc, "key", 0))
      return 0;

   if (! parse_key(args[0], &key))
      return 0;

   binding_del_binding(context.current_mode->root, &key);
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
         write_error("%d:%d: unknown command: %s", lex_line, lex_line_pos, args[0]);
         ret = -1;
         goto END;
      }
      else if (! conf_command(nargs - 1, &args[1])) {
         prepend_error("%d:%d: %s", lex_line, lex_line_pos, expaned_name);
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
