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

/*
 * default-action: drop|pass (ignore unkown keys, pass keys as keystroke)
 * mode "blah": create new mode
 * switch-to
 *
 *
 * [normal]
 *    bind i switch-to insert
 *
 *    bind raw(
 *
 * [insert]
 *    default-action:
 *    bind 33 switch-to normal
 *    bind 'soundso' type "asfsdfsf"; blahblah
 *
 */

int main()
{
   initscr();
   cbreak();
   noecho();
   nonl();
   intrflush(stdscr, FALSE);
   keypad(stdscr, TRUE);
   endwin();


   const char *seq = keybound(KEY_PPAGE, 0);
   printf("\n> %s\n", keybound(KEY_PPAGE, 0));

   while (*seq) {
      printf("\n>%c", *seq + '0');
      ++seq;
   }
}
