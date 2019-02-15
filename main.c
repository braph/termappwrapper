#include "iwrap.h"
#include "conf.h"
#include "termkeystuff.h"
#include "vi_conf.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

#include <pty.h>
#include <term.h>
#include <termios.h>
#include <ncurses.h>

#define USAGE \
"Usage: %s [OPTIONS] PROGRAM [ARGUMENTS...]\n\n" \
"OPTIONS\n\n"                                    \
" -c file\t"    "Read config file\n"             \
" -C string\t"  "Read config string\n"           \
" -m mode\t"    "Start in mode\n"                \
" -v\t\t"       "Load vi config\n"
#define GETOPT_OPTS "+c:C:m:hv"

/*
 * TODO: repeat-max
 * TODO: instant-leave mode?
 * TODO: unbind, bind check for duplicate key bind
 * TODO: resize pty
 */

void  cleanup();
void *redirect_to_stdout();
int   forkapp(char **, int*, pid_t*);

pthread_t      redir_thread;
struct termios restore_termios;

int main(int argc, char *argv[]) {
   int         c;
   char        *mode = NULL;

   context_init();

   if (! load_terminfo())
      errx(1, "Could not setup terminfo");

   if (! (tk = termkey_new_abstract(getenv("TERM"), 0)))
      err(1, "Initializing termkey failed");

   while ((c = getopt(argc, argv, GETOPT_OPTS)) != -1)
      #define case break; case
      switch (c) {
      case 'h':
         printf(USAGE"\n", argv[0]);
         return 0;
      case 'c':
         if (read_conf_file(optarg) < 0)
            errx(1, "Option -c '%s': %s", optarg, get_error());
      case 'C': 
         if (read_conf_string(optarg) < 0)
            errx(1, "Option -C '%s': %s", optarg, get_error());
      case 'v':
         if (read_conf_string(VI_CONF) < 0)
            errx(1, "%s", get_error());
      case 'm':
         mode = optarg;
      case '?':
         return 1;
      }
      #undef case

   if (optind == argc)
      errx(1, "Missing command");

   if (mode) {
      if (! (context.current_mode = get_keymode(mode)))
         errx(1, "Option -m: unknown mode: %s", mode);
   }
   else {
      context.current_mode = &context.global_mode;
   }

   if (forkapp(&argv[optind], &context.program_fd, &context.program_pid) < 0)
      err(1, "Could not start process");

   if ((errno = pthread_create(&redir_thread,
         NULL, redirect_to_stdout, (void*)&context.program_fd)))
      err(1, "Starting thread failed");

   char      buf[32];
   int       bufi        = 0;
   const int escdelay_ms = 10;

   struct pollfd fds[2] = {
      { .fd = STDOUT_FILENO,      .events = POLLIN },
      { .fd = context.program_fd, .events = POLLIN }
   };

   TermKeyKey key;
   TermKeyKey escape = {
      .type      = TERMKEY_TYPE_KEYSYM,
      .code.sym  = TERMKEY_SYM_ESCAPE,
      .modifiers = 0
   };

   struct termios termios;
   if (tcgetattr(STDIN_FILENO, &termios) == 0) {
      restore_termios      = termios;
      termios.c_iflag     &= ~(IXON|INLCR|ICRNL);
      termios.c_lflag     &= ~(ICANON|ECHO);
      termios.c_cc[VMIN]   = 1;
      termios.c_cc[VTIME]  = 0;
      termios.c_lflag     &= ~ISIG;
      tcsetattr(STDIN_FILENO, TCSANOW, &termios);
   }

   atexit(cleanup);
   signal(SIGINT, cleanup);
   signal(SIGTERM, cleanup);

   setbuf(stdin, NULL);
   for (;;) {
      for (;;) {
         poll(fds, 2, -1);

         if (fds[1].revents & POLLHUP || fds[1].revents & POLLERR) {
            int ret;
            waitpid(context.program_pid, &ret, 0);
            return WEXITSTATUS(ret);
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

void cleanup() {
   tcsetattr(STDIN_FILENO, TCSANOW, &restore_termios);
#if FREE_MEMORY
   termkey_destroy(tk);
   context_free();
   unload_terminfo();
   pthread_join(redir_thread, NULL);
#endif
   exit(0);
}

void *redirect_to_stdout(void *_fd)
{
   #define  REDIRECT_BUFSZ 4096
   int      fd = *((int*)_fd);
   char     buffer[REDIRECT_BUFSZ];
   char     *b;
   ssize_t  bytes_read;
   ssize_t  bytes_written;

   for (;;) {
      if ((bytes_read = read(fd, &buffer, REDIRECT_BUFSZ)) == -1) {
         if (errno == EAGAIN) {
            usleep(100);
            continue;
         }
         else {
            return NULL;
         }
      }

      b = buffer;
      for (int i = 0; i < 5; i++) {
         bytes_written = write(STDOUT_FILENO, b, bytes_read);

         if (bytes_written >= 0) {
            b += bytes_written;
            bytes_read -= bytes_written;
            if (bytes_read == 0)
               break;
         }
         else if (bytes_written == -1 && errno != EAGAIN)
            break;

         usleep(100);
      }
   }
}

int forkapp(char **argv, int *ptyfd, pid_t *pid) {
   struct termios tios;
   struct winsize winsz;

   tcgetattr(0, &tios);
   ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsz);
      
   *pid = forkpty(ptyfd, NULL, &tios, &winsz);

   if (*pid < 0)
      return -1;
   else if (*pid == 0) {
      execvp(argv[0], &argv[0]);
      return -1;
   }

   return 1;
}
