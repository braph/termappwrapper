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
#include <termios.h>
#include <readline/readline.h>

/*
 * NOTE: This command only works right in programs that can react to the SIGWINCH singal
 */

typedef struct cmd_readline_args {
   bool  no_newline;
   char  *prompt;
   char  *init;
} cmd_readline_args;

static cmd_readline_args *current_arg;
extern rl_hook_func_t *rl_startup_hook;

int rl_set_init_text(void) {
   if (current_arg->init)
      rl_insert_text(current_arg->init);

   return 0;
}

static
void readline_call(command_call_t *cmd, TermKeyKey *key) {
   int x, y;
   char *line;
   rl_startup_hook = &rl_set_init_text;
   current_arg     = (cmd_readline_args*) cmd->arg;

   stop_program_output();

   program_cursor(context.program_fd, &x, &y);
   printf("> %d, %d\n", x, y);

   tcsetattr(STDIN_FILENO, TCSANOW, &context.tios_restore);
   line = readline(arg->prompt);

   tcsetattr(STDIN_FILENO, TCSANOW, &context.tios_wrap);
   start_program_output();

   kill(context.program_fd, SIGWINCH);

   if (line && strlen(line) > 0) {
      write_to_program(line);
      if (! current_arg->no_newline)
         write_to_program("\r");
      free(line);
   }
}

static
void* cmd_readline_parse(int argc, char *args[], option *options) {
   cmd_readline_args *cmd_args = calloc(1, sizeof(*cmd_args));

   for (option *opt = options; opt->opt; ++opt) {
      if (opt->opt == 'p')
         cmd_args->prompt = strdup(opt->arg);
      else if (opt->opt == 'i')
         cmd_args->init = strdup(opt->arg);
      else if (opt->opt == 'n')
         cmd_args->no_newline = 1;
   }

   return (void*) cmd_args;
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
      {'n', NULL,     "Do not output a newline"},
      {0,0,0} },
   .parse = &cmd_readline_parse,
   .call  = &readline_call,
   .free  = &cmd_readline_free
};

