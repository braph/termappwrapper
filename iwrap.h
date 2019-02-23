#ifndef _IWRAP_H
#define _IWRAP_H

#include <stdint.h>
#include <sys/types.h>
#include <termios.h>
#include <pthread.h>
#include <termkey.h>
#include "options.h"

struct context_t;
struct binding_t;
struct command_call_t;

// === Global variables ===
TermKey            *tk;
struct context_t   context;
// ========================

char* get_error();
void  write_error(const char *fmt, ...);
void  prepend_error(const char *fmt, ...);

typedef struct command_opt_t {
   const char  opt;
   const char *meta;
   const char *desc;
} command_opt_t;

typedef struct command_t {
   const char          *name;
   const char          *desc;
   const char         **args;
   const command_opt_t *opts;
   void*              (*parse) (int, char **, option*);
   void               (*call)  (struct command_call_t*, TermKeyKey*);
   void               (*free)  (void*);
} command_t;

typedef struct command_call_t {
   struct command_t *command;
   void             *arg;
} command_call_t;

#define BINDING_TYPE_COMMAND 0
#define BINDING_TYPE_CHAINED 1
typedef struct binding_t {
   TermKeyKey  key;
   uint16_t    type :  1;
   uint16_t    size : 15;
   union {
      struct command_call_t *commands;
      struct binding_t     **bindings;
   } p;
} binding_t;

void binding_free(binding_t *);
binding_t* binding_get_binding(binding_t *, TermKeyKey *);
binding_t* binding_add_binding(binding_t *, binding_t *);

typedef struct keymode_t {
   char        *name;
   uint16_t    repeat          :  1;
   uint16_t    ignore_unmapped :  1;
   uint16_t    n_bindings      : 14;
   binding_t   **bindings;
} keymode_t;

void       keymode_init(keymode_t*, const char*);
void       keymode_free(keymode_t*);
void       keymode_add_binding(keymode_t*, binding_t*);
binding_t* keymode_get_binding(keymode_t*, TermKeyKey*);
void       keymode_del_binding(keymode_t*, TermKeyKey*);
keymode_t* get_keymode(char *name);
keymode_t* add_keymode(char *name);

typedef struct context_t {
   int            program_fd;
   pid_t          program_pid;

   uint32_t       mask        :  1;
   uint32_t       stop_output :  1;
   uint32_t       n_keymodes  : 12;
   uint32_t       repeat      : 18;

   keymode_t      global_mode;
   keymode_t      **keymodes;
   keymode_t      *current_mode;
   binding_t      *current_binding;

   struct termios tios_restore;
   pthread_t      redir_thread;
} context_t;

void context_init();
void context_free();

void  handle_key(TermKeyKey *key, char *raw, int len);
void  write_to_program(char *);
int   check_args(int argc, ...);
int   check_args_new(int argc, const char *args[]); 
char* args_get_arg(int *, char***, const char*);
int   start_program_output();
void  stop_program_output();

void  set_input_mode();
void  get_cursor(int fd, int *x, int *y);
void  set_cursor(int fd, int x, int y);
void  get_winsize(int fd, int *x, int *y);

#endif
