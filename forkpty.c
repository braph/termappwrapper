#include <err.h>
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <pty.h>
#include <termios.h>
#include <fcntl.h>
#include <pthread.h>
#include <ncurses.h>
#include <term.h>

void *redirect_to_stdout(void *_fd)
{
   int fd = *((int*)_fd);
   #define REDIRECT_BUFSZ 1024
   char buffer[REDIRECT_BUFSZ];
   ssize_t bytes_read;

   for (;;) {
      if ((bytes_read = read(fd, &buffer, REDIRECT_BUFSZ)) != -1)
         write(STDOUT_FILENO, &buffer, bytes_read);
   }
}

int main()
{
   int master;
   pid_t pid;

   struct termios tios;
   struct winsize winsz;

   tcgetattr(master, &tios);
   ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsz);
      
   pid = forkpty(&master, NULL, &tios, &winsz);

   if (pid < 0) {
      return 1;
   }
   else if (pid == 0) {
      char *args[] = { NULL };
      execvp("/usr/bin/ncmpcpp", args);
      err(1, "could not start process");
   }
   else {
      pthread_t redir_thread;
      pthread_create(&redir_thread, NULL, redirect_to_stdout, (void*)&master);

      initscr();
      cbreak();
      noecho();
      nonl();
      intrflush(stdscr, FALSE);
      keypad(stdscr, TRUE);

      //for (int i = 0; i < 
      for (char **it = cur_term->type.Strings; *it; ++it)
         warnx("%x", *it);

      int input;
      for (;;) {
         input = getch();
         write(master, &input, 1);
      }
   }

   return 0;
}
