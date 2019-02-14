#ifndef _IWRAP_H
#define _IWRAP_H

#include <stdbool.h>
#include <termkey.h>

struct command_call_t;
struct context_t;

// === Global variables ===
TermKey            *tk;
struct context_t   context;
// ========================

char* get_error();
void  write_error(const char *fmt, ...);
void  prepend_error(const char *fmt, ...);

typedef struct command_t {
   const char   *name;
   void*       (*parse) (int, char **);
   void        (*call)  (struct command_call_t*, TermKeyKey*);
   void        (*free)  (void*);
} command_t;

typedef struct command_call_t {
   struct command_t *command;
   void             *arg;
} command_call_t;

typedef struct binding_t {
   TermKeyKey      *key;
   command_call_t  *commands;
   int             n_commands;
} binding_t;

void binding_free(binding_t *binding);

typedef struct keymode_t {
   char        *name;
   bool        repeat;
   int         ignore_unmapped;
   int         n_bindings;
   binding_t   **bindings;
} keymode_t;

void       keymode_init(keymode_t*, const char*);
void       keymode_free(keymode_t*);
void       keymode_add_binding(keymode_t*, binding_t*);
binding_t* keymode_get_binding(keymode_t*, TermKeyKey*);
keymode_t* get_keymode(char *name);
keymode_t* add_keymode(char *name);

typedef struct context_t {
   bool        mask;
   int         repeat;
   int         program_fd;
   pid_t       program_pid;
   keymode_t   **keymodes;
   int         n_keymodes;
   keymode_t   *current_mode;    
   keymode_t   global_mode;
} context_t;

void context_init();
void context_free();

void  handle_key(TermKeyKey *key, char *raw, int len);
void  write_to_program(char *);
int   check_args(int argc, ...);
void* cmd_parse_none(int, char **);

#endif
