#include "iwrap.h"
#include "termkeystuff.h"
#include "common.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

#define      IWRAP_ERROR_SIZE 2048
static char  iwrap_error[IWRAP_ERROR_SIZE];

char *get_error() {
   return iwrap_error;
}

void write_error(const char *fmt, ...) {
   va_list ap;
   va_start(ap, fmt);
   vsnprintf(iwrap_error, IWRAP_ERROR_SIZE, fmt, ap);
   va_end(ap);
}

void prepend_error(const char *fmt, ...) {
   char temp[IWRAP_ERROR_SIZE];
   va_list ap;
   va_start(ap, fmt);
   vsnprintf(temp, IWRAP_ERROR_SIZE, fmt, ap);
   snprintf(temp+strlen(temp), IWRAP_ERROR_SIZE-strlen(temp), ": %s", iwrap_error);
   strncpy(iwrap_error, temp,  IWRAP_ERROR_SIZE);
   va_end(ap);
}

void write_to_program(char *s) {
   int      i;
   ssize_t	n, slen;

   slen = strlen(s);
   for (i = 0; i < 5; i++) {
      n = write(context.program_fd, s, slen);
      if (n >= 0) {
         s += n;
         slen -= n;
         if (slen == 0)
            break;
      } else if (n == -1 && errno != EAGAIN)
         break;
      usleep(100);
   }
}

void context_init() {
   context.keymodes                    = NULL;
   context.n_keymodes                  = 0;
   context.current_mode                = &context.global_mode;
   context.mask                        = 0;
   context.repeat                      = 0;
   keymode_init(&context.global_mode, "global");
}

#if FREE_MEMORY
void context_free() {
   keymode_free(&context.global_mode);

   for (int i=0; i < context.n_keymodes; ++i) {
      keymode_free(context.keymodes[i]);
      free(context.keymodes[i]);
   }

   free(context.keymodes);
}
#endif


void keymode_init(keymode_t *km, const char *name) {
   km->name            = strdup(name);
   km->bindings        = NULL;
   km->n_bindings      = 0;
   km->ignore_unmapped = 0;
   km->repeat          = 0;
}

keymode_t* get_keymode(char *name) {
   if (streq(name, "global"))
      return &context.global_mode;

   for (int i=0; i < context.n_keymodes; ++i)
      if (streq(name, context.keymodes[i]->name))
         return context.keymodes[i];

   return NULL;
}

keymode_t* add_keymode(char *name) {
   keymode_t *km = malloc(sizeof(*km));
   keymode_init(km, name);

   context.n_keymodes++;
   context.keymodes = realloc(context.keymodes, context.n_keymodes * sizeof(km));
   context.keymodes[context.n_keymodes - 1] = km;
   return km;
}

void keymode_add_binding(keymode_t *km, binding_t *binding) {
   km->n_bindings++;
   km->bindings = realloc(km->bindings, km->n_bindings * sizeof(binding_t*));
   km->bindings[km->n_bindings - 1] = binding;
}

binding_t*
keymode_get_binding(keymode_t *km, TermKeyKey *key) {
   for (int i=0; i < km->n_bindings; ++i)
      if (! termkey_keycmp(tk, key, km->bindings[i]->key))
         return km->bindings[i];

   return NULL;
}

#if FREE_MEMORY
void keymode_free(keymode_t *km) {
   for (int i=0; i < km->n_bindings; ++i) {
      binding_free(km->bindings[i]);
      free(km->bindings[i]);
   }

   free(km->name);
   free(km->bindings);
}
#endif


void command_call_free(command_call_t *call) {
   if (call->command->free)
      call->command->free(call->arg);
}

void binding_free(binding_t *binding) {
   free(binding->key);

   for (int i=0; i < binding->n_commands; ++i)
      command_call_free(&binding->commands[i]);
   free(binding->commands);
}

void command_execute(command_call_t *cmd, TermKeyKey *key) {
   cmd->command->call(cmd, key);
}

void commands_execute(binding_t *binding, TermKeyKey *key) {
   for (int i=0; i < binding->n_commands; ++i)
      command_execute(&binding->commands[i], key);
}

int check_args(int argc, ...) {
   va_list ap;
   char   *arg;
   va_start(ap, argc);
   #define return va_end(ap); return

   for (;;) {
      arg = va_arg(ap, char *);

      if (arg == 0) {
         if (argc) {
            write_error("spare arguments");
            return 0;
         }

         break; // OK
      }
      else if (*arg == '+') {
         if (! argc) {
            write_error("missing argument: <%s>", &arg[1]);
            return 0;
         }

         break; // OK
      }
      else if (*arg == '*') {
         break; // OK
      }
      else {
         if (! argc--) {
            write_error("missing argument: <%s>", arg);
            return 0;
         }
      }
   }

   return 1;
   #undef return
}

void handle_key(TermKeyKey *key, char *raw, int len) {
   if (context.mask) {
      context.mask = 0;
      goto WRITE_RAW;
   }

   /*
   if (key->type == TERMKEY_TYPE_UNICODE) {
      if (key->code.codepoint == 's') {
   */

   // Special case: If building key repetition, don't pass 0 as keybinding,
   // instead multiply our current repeat val by 10
   if (context.repeat                    &&
       context.current_mode->repeat      &&
       key->type == TERMKEY_TYPE_UNICODE &&
       key->code.codepoint == '0') {
      context.repeat *= 10;
      return;
   }

   // === Try current_mode then global_mode ===================================
   keymode_t *keymode = context.current_mode;
   binding_t *binding;

   NEXT_KEYMODE:
   binding = keymode_get_binding(keymode, key);

   if (binding) {
      if (keymode->repeat && context.repeat) {
         for (int r = 0; r < context.repeat; ++r)
            commands_execute(binding, key);
         context.repeat = 0;
      }
      else {
         commands_execute(binding, key);
      }

      return;
   }

   if (keymode != &context.global_mode) {
      keymode = &context.global_mode;
      goto NEXT_KEYMODE;
   }
   // =========================================================================

   if (context.current_mode->repeat) {
      if (key->type == TERMKEY_TYPE_UNICODE &&
          key->code.codepoint >= '1'        &&
          key->code.codepoint <= '9')
      {
         context.repeat = context.repeat * 10 + (key->code.codepoint - 0x30);
         return;
      }
      else
         context.repeat = 0;
   }

   /*
   char *s = get_key_code(key);
   if   (s)  write(context.program_fd, s, strlen(s));
   fprintf(stderr, "%10s: ", format_key(key));
   for (int i = 0; i < len; ++i) fprintf(stderr, " %3d", raw[i]);
   for (int i = len; i < 9; ++i) fprintf(stderr, " %3d", 0);
   fprintf(stderr, "\n");
   */

   /*
   printf("ign=%d, type=%d resul=%d\n",
         context.current_mode->ignore_unmapped,
         TERMKEY_TYPE_TO_FLAG(key->type),
         context.current_mode->ignore_unmapped & TERMKEY_TYPE_TO_FLAG(key->type));
   */

   if (context.current_mode->ignore_unmapped & TERMKEY_TYPE_TO_FLAG(key->type)) {
      // Modified unicode is considered as a keysym
      if (key->type != TERMKEY_TYPE_UNICODE || key->modifiers == 0) {
         return;
      }
   }

   WRITE_RAW:
   write(context.program_fd, raw, len);
}

void* cmd_parse_none(int argc, char **args) {
   if (! check_args(argc, 0))
      return NULL;
   return (void*)1;
}
