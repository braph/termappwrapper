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

int print_string_literal(char *s)
{
   putchar('\"');

   while (*s) {
      unsigned cp = (unsigned char)*s++;
      printf("\\x%.2x", cp);
   }

   putchar('\"');
}

int main()
{
   initscr();
   cbreak();
   noecho();
   nonl();
   intrflush(stdscr, FALSE);
   keypad(stdscr, TRUE);

   char *s;

   int i = 0;
   while (strfnames[i]) {
      printf("%s: %s %s -- ", strfnames[i], strnames[i], strcodes[i]);

      s = tigetstr(strnames[i]);

      if (s != (char *) -1 && s != NULL) 
         print_string_literal(s);
         //printf("%s", s);

      printf("\n");
      ++i;
   }
}
