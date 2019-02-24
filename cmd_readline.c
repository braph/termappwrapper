#include "iwrap.h"
#include "termkeystuff.h"
#include "common.h"
#include "options.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>

#include <pty.h>
#include <termios.h>
#include <readline/readline.h>

typedef struct cmd_readline_args {
   uint8_t  no_newline : 1;
   uint8_t  no_clear   : 1;
   uint8_t  no_refresh : 1;
   uint8_t  __PAD__    : 5;
   int16_t x;
   int16_t y;
   char  *prompt;
   char  *init;
} cmd_readline_args;

static cmd_readline_args *current_arg;
extern rl_hook_func_t *rl_startup_hook;

int rl_set_init_text(void) {
   rl_reset_terminal(getenv("TERM"));
   if (current_arg->init)
      rl_insert_text(current_arg->init);
   return 0;
}

static
void get_winsize(int fd, int *y, int *x) {
   struct winsize wsz;
   if (ioctl(fd, TIOCGWINSZ, &wsz) != -1) {
      *x = wsz.ws_row;
      *y = wsz.ws_col;
   }
}

static
void refresh_window(int fd) {
   struct winsize wsz;
   if (ioctl(fd, TIOCGWINSZ, &wsz) != -1) {
      wsz.ws_col--;
      ioctl(fd, TIOCSWINSZ, &wsz);
      wsz.ws_col++;
      ioctl(fd, TIOCSWINSZ, &wsz);
   }
}

static
void readline_call(command_call_t *cmd, TermKeyKey *key) {
   int old_x, old_y, max_x = 0, max_y = 0, x, y;
   char *line;
   cmd_readline_args *args = cmd->arg;

   rl_startup_hook = &rl_set_init_text;
   current_arg = args;

   stop_program_output();

   struct termios tios = context.tios_restore;
   //tios.c_lflag    &= ~(ECHONL|ECHO);
   //tios.c_oflag    &= ~(ONLRET);
   tcsetattr(STDOUT_FILENO, TCSANOW, &tios);

   //tcsetattr(STDOUT_FILENO, TCSANOW, &context.tios_restore);

   get_cursor(context.program_fd, &old_y, &old_x);
   get_winsize(context.program_fd, &max_y, &max_x);

   if (args->x < 0) {
      x = max_x + (args->x + 1);
      if (x < 1)
         x = 1;
   }
   else
      x = (args->x > max_x ? max_x : args->x);

   if (args->y < 0) {
      y = max_y + (args->y + 1);
      if (y < 1)
         y = 1;
   }
   else
      y = (args->y > max_y ? max_y : args->y);

   set_cursor(STDOUT_FILENO, y, x);

   if (! args->no_clear)
      write(STDOUT_FILENO, "\033[K", 3);

   line = readline(args->prompt);

   set_input_mode();
   start_program_output();
   set_cursor(STDOUT_FILENO, old_y, old_x);

   if (! args->no_refresh)
      refresh_window(context.program_fd);

   if (line && strlen(line) > 0) {
      write_to_program(line);
      if (! args->no_newline)
         write_to_program("\r");
      free(line);
   }
}

static
void* cmd_readline_parse(int argc, char *args[], option *options) {
   cmd_readline_args *cmd_args = calloc(1, sizeof(*cmd_args));
   cmd_args->x = 1;
   cmd_args->y = -1;

   for (option *opt = options; opt->opt; ++opt) {
      if (opt->opt == 'p')
         cmd_args->prompt = strdup(opt->arg);
      else if (opt->opt == 'i')
         cmd_args->init = strdup(opt->arg);
      else if (opt->opt == 'n')
         cmd_args->no_newline = 1;
      else if (opt->opt == 'C')
         cmd_args->no_clear = 1;
      else if (opt->opt == 'R')
         cmd_args->no_refresh = 1;
      else if (opt->opt == 'x') {
         if (! (cmd_args->x = atoi(opt->arg))) {
            write_error("invalid value");
            goto ERROR;
         }
      }
      else if (opt->opt == 'y') {
         if (! (cmd_args->y = atoi(opt->arg))) {
            write_error("invalid value");
            goto ERROR;
         }
      }
   }

   return (void*) cmd_args;

ERROR:
   free(cmd_args);
   return NULL;
}

static
void cmd_readline_free(void *_arg) {
   cmd_readline_args *arg = (cmd_readline_args*) _arg;
   free(arg->prompt);
   free(arg->init);
   free(arg);
}

command_t command_readline = {
   .name  = "readline",
   .desc  = "Open a readline",
   .args  = NULL,
   .opts  = (const command_opt_t[]) {
      {'p', "PROMPT", "Set prompt"},
      {'i', "INIT",   "Set initial text"},
      {'x', "X",      "Set x cursor postion (starting from left - use negative value to count from right)"},
      {'y', "Y",      "Set y cursor postion (starting from top - use negative value to count from bottom)"},
      {'n', NULL,     "Do not append a newline to the user input"},
      {'C', NULL,     "Do not clear the line"},
      {'R', NULL,     "Do not refresh the window"},
      {0,0,0}
   },
   .parse = &cmd_readline_parse,
   .call  = &readline_call,
   .free  = &cmd_readline_free
};

