// c++ stdlib
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>

// c
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// other
#include <term.h>
#include <ncurses.h>

using namespace std;

map<string, int> curs_keycodes;
map<string, string> escape_sequences;

void fill_alt_keycodes() {
   int         ch;
   int         curs_keycode;
   char        key_sequence[3];

   curs_keycode = KEY_MAX + 1;
   key_sequence[0] = 27;
   key_sequence[2] = 0;

   for (ch = 20; ch < 127; ++ch) {
      key_sequence[1] = ch;

      if (ERR == define_key(key_sequence, curs_keycode)) {
         errx(1, "define_key()");
      }

      ++curs_keycode;
   }
}

void fill_curs_keycodes() {
   int            key;
   const char     *kname;
   const char     *it;
   string         kname_str;

   for (key = KEY_MIN; key != KEY_MAX; ++key) {
      kname = keyname(key);

      if (! kname)
         continue;
      if (! strcmp("UNKNOWN KEY", kname))
         continue;

      kname_str.clear();

      for (it = kname + 4; *it; ++it) {
         // ignore brackets in KEY_F(1), KEY_F(2), ...
         if (*it == '(' || *it == ')') {
         }
         else { // transform to lower case
            kname_str += (char) tolower(*it);
         }
      }

      curs_keycodes[kname_str] = key;
   }
}

void fill_console_codes() {
   char        *sequence;
   int         i = 0;
   string      kname_str;

   while (strfnames[i]) {
      //printf("having strfname=%s\n", strfnames[i]);

      if (! strncmp("key_", strfnames[i], 4)) {
         sequence = tigetstr(strnames[i]);

         if (sequence != 0 && sequence != (char *) -1) {
            kname_str = strfnames[i] + 4;
            escape_sequences[kname_str] = sequence;
            //printf("%s: %s %s -- ", strfnames[i], strnames[i], strcodes[i]);
            //printf("%s", s);
            //printf("\n");
            //cout << kname_str << " => " << sequence << '\n';
         }
      }
      
      ++i;
   }
}

void dump_tables() {
   cout << "curs_keycodes:" << endl;
   for (map<string, int>::iterator it = curs_keycodes.begin(); it != curs_keycodes.end(); ++it) {
      cout << it->first << " => " << it->second << '\n';
   }

   cout << "escape_sequences:" << endl;
   for (map<string, string>::iterator it = escape_sequences.begin(); it != escape_sequences.end(); ++it) {
      cout << it->first << " => " << it->second << '\n';
   }
}

/*
//std::map<int, Actions> root_bindings;

// bind -m normal KEY_ESCAPE switch_mode sdfasdf

class Action
{
};

class TypeAction : public Action
{
   string m_TypeString;

   void execute()
   {
   }
};

class SwitchModeAction : public Action
{
   string m_Mode;

   void execute()
   {
   }
};

typedef vector<Action> Actions;

class Mapping
{
   map<int, Actions>;
}

class Modes
{
   int m_currentMode;


   void switchMode(int mode)
   {
   }
};
*/

extern FILE *yyin;

void read_config(const char *file) {
   FILE *fh = fopen(file, "r");

   //yyin = fh;
}

int main(int argc, char *argv[]) {
   //setupterm((char *)0, 1, (int *)0);
   initscr();

   printf("fill_curs_keycodes\n");
   fill_curs_keycodes();

   printf("fill_console_codes\n");
   fill_console_codes();

   fill_alt_keycodes();

   printf("read_config\n");
   //read_config("./cfg.cfg");

   //dump_tables();

   initscr();
   cbreak();
   noecho();
   nonl();
   intrflush(stdscr, FALSE);
   keypad(stdscr, TRUE);

   //endwin();

   int input;
   int count = 0;

   for (;;) {
      input = getch();
      printf("input is %d\n", input);

      int keycount;
      char *sequence;
      for (keycount = 0;keycount < 1000; ++keycount) {
         if (NULL != (sequence = keybound(input, keycount))) {
            printf("key escape is: ");
            for (char *it = sequence; *it; ++it) {
               printf("%d", *it);
            }
            printf("\n");
         }
      }

      if (input >= '0' && input <= '9') {
         count = count * 10 + (input - '0');
      }
      else if (input == 033) {
         if (count)
            count = 0;
         else
            printf("escape\n");
      }
      else {
      }

      printf("count = %d\n", count);
   }
}

