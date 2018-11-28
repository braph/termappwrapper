#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <term.h>

int main()
{
   initscr();
   noecho();
   keypad(stdscr, TRUE);

   int ch;
   char key[3] = "\e ";

   /* define alt combinations: ESC + character */
   for (ch = 32; ch <= 1270; ++ch) {
      key[1] = ch;
      define_key(key, KEY_MAX + ch);
   }

   for (int i = KEY_MIN; i < KEY_MAX; ++i)
      keyname(i);

   /* print received keycodes */
   for (;;) {
      ch = getch();

      if (ch == 13) {
         addch('\n'); continue;
      }

      char buf[10];
      sprintf(buf, "%d", ch);
      addstr(buf);
      addch(' ');
   }
}
