#include "iwrap.h"
#include "conf.h"
#include "termkeystuff.h"
#include "vi_conf.h"
#include "commands.h"

#include <err.h>
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

#include <pty.h>
#include <termios.h>
//#include <term.h>
//#include <ncurses.h>

#define USAGE \
"Usage: %s [OPTIONS] PROGRAM [ARGUMENTS...]\n\n" \
"OPTIONS\n\n"                                    \
" -c file\t"                  "Read config file\n"             \
" -C string\t"                "Read config string\n"           \
" -m mode\t"                  "Start in mode\n"                \
" -b key cmd\t"               "Alias for 'bind key cmd'\n"     \
" -k <key in> <key out>"      "Alias for 'bind key_in key key_out'\n" \
" -v\t\t"                     "Load vi config\n"
#define GETOPT_OPTS "+c:C:m:b:k:hv"

/* TODO: repeat-max
 * TODO: instant-leave mode? */

int   help(char *);
void  cleanup();
void  update_pty_size(int);
void  sighandler(int);
int   forkapp(char **, int*, pid_t*);

int main(int argc, char *argv[]) {
   printf("%d\n", sizeof(context));

   int         c;
   char        *mode   = "global";
   char        *cmdbuf = NULL;
   const char  *arg2   = NULL;

   context_init();

   if (! load_terminfo())
      errx(1, "Could not setup terminfo");

   if (! (tk = termkey_new_abstract(getenv("TERM"), 0)))
      err(1, "Initializing termkey failed");

   while ((c = getopt(argc, argv, GETOPT_OPTS)) != -1)
      #define case break; case
      switch (c) {
      case 'h':
         return help(argv[0]);
      case 'm':
         mode = optarg;
      case 'c':
         if (read_conf_file(optarg) < 0)
            errx(1, "Option -c '%s': %s", optarg, get_error());
      case 'C': 
         if (read_conf_string(optarg) < 0)
            errx(1, "Option -C '%s': %s", optarg, get_error());
      case 'b':
         cmdbuf = realloc(cmdbuf, sizeof("bind.") + strlen(optarg));
         sprintf(cmdbuf, "bind %s", optarg);
         if (read_conf_string(cmdbuf) < 0)
            errx(1, "Option -b '%s': %s", optarg, get_error());
      case 'k':
         if ((arg2 = argv[++optind]) == NULL)
            errx(1, "Option -k '%s': Missing argument", optarg);
         cmdbuf = realloc(cmdbuf, sizeof("bind..key.") + strlen(optarg) + strlen(arg2));
         sprintf(cmdbuf, "bind %s key %s", optarg, arg2);
         if (read_conf_string(cmdbuf) < 0)
            errx(1, "Option -k '%s' '%s': %s", optarg, arg2, get_error());
      case 'v':
         mode = "vi";
         if (read_conf_string(VI_CONF) < 0)
            errx(1, "%s", get_error());
      case '?':
         return 1;
      }
      #undef case
   free(cmdbuf);

   if (optind == argc)
      errx(1, "Missing command");

   if (! (context.current_mode = get_keymode(mode)))
      errx(1, "Option -m: unknown mode: %s", mode);

   if (forkapp(&argv[optind], &context.program_fd, &context.program_pid) < 0)
      err(1, "Could not start process");

   if (! start_program_output())
      err(1, "Starting thread failed");

   if (getenv("TMUX") && fork() == 0) {
      close(0);
      close(1);
      close(2);
      return execlp("tmux", "tmux", "setw", "escape-time", "50", NULL);
   }

   #define tios context.tios_wrap
   if (tcgetattr(STDIN_FILENO, &tios) == 0) {
      context.tios_restore = tios;
      tios.c_iflag    |= IGNBRK;
      tios.c_iflag    &= ~(IXON|INLCR|ICRNL);
      tios.c_lflag    &= ~(ICANON|ECHO|ECHONL|ISIG);
      tios.c_oflag    &= ~(OPOST|ONLCR|OCRNL|ONLRET);
      tios.c_cc[VMIN]  = 1;
      tios.c_cc[VTIME] = 0;
      tcsetattr(STDIN_FILENO, TCSANOW, &tios);
   }
   #undef tios

   atexit(cleanup);
   signal(SIGINT,   sighandler);
   signal(SIGTERM,  sighandler);
   signal(SIGWINCH, update_pty_size);
   setbuf(stdin, NULL);

   #define   bufsz 16
   char      buf[bufsz];
   int       bufi;
   #define   escdelay_ms 10

   struct pollfd fds[2] = {
      { .fd = STDIN_FILENO,       .events = POLLIN },
      { .fd = context.program_fd, .events = POLLIN }
   };

   TermKeyKey key;
   TermKeyKey escape = {
      .type      = TERMKEY_TYPE_KEYSYM,
      .code.sym  = TERMKEY_SYM_ESCAPE,
      .modifiers = 0
   };

   bufi = 0;
   for (;;) {
      for (;;) {
         poll(fds, 2, -1);

         if (fds[1].revents & POLLHUP || fds[1].revents & POLLERR) {
            #define return_val c
            waitpid(context.program_pid, &return_val, 0);
            return WEXITSTATUS(return_val);
         }

         if (fds[0].revents & POLLIN)
            break;
      }

      c = getchar();
      buf[bufi++] = c;

      if (c == 033) {
         if (poll(fds, 1, escdelay_ms) > 0 && fds[0].revents & POLLIN)
            goto NON_ESCAPE;
         else {
            handle_key(&escape, buf, bufi);
            bufi = 0;
            continue;
         }
      }

      NON_ESCAPE:
      termkey_push_bytes(tk, (char*) &c, 1);
      if (termkey_getkey(tk, &key) == TERMKEY_RES_KEY) {
         handle_key(&key, buf, bufi);
         bufi = 0;
      }
   }

   return 0;
}

int help(char *prog) {
   printf(USAGE"\n", prog);

   printf("Available commands:\n");
   for (int i = 0; i < COMMANDS_SIZE; ++i) {
      printf("  ");
      fprint_command_usage(stdout, commands[i]);
      printf("\n");
   }

   return 0;
}

void cleanup() {
   tcsetattr(STDIN_FILENO, TCSANOW, &context.tios_restore);
#if FREE_MEMORY
   termkey_destroy(tk);
   context_free();
   unload_terminfo();
   stop_program_output();
   pthread_join(context.redir_thread, NULL);
#endif
   exit(0);
}

void sighandler(int sig) {
   signal(sig, SIG_DFL);
   exit(0);
}

int forkapp(char **argv, int *ptyfd, pid_t *pid) {
   struct winsize wsz;
   struct termios tios;

   tcgetattr(STDIN_FILENO, &tios);
   ioctl(STDIN_FILENO, TIOCGWINSZ, &wsz);
      
   *pid = forkpty(ptyfd, NULL, &tios, &wsz);

   if (*pid < 0)
      return -1;
   else if (*pid == 0) {
      execvp(argv[0], &argv[0]);
      return -1;
   }

   return 1;
}

void update_pty_size(int _) {
   struct winsize ws;
   if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) != -1)
      ioctl(context.program_fd, TIOCSWINSZ, &ws);
}
