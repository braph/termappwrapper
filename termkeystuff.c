#include "termkeystuff.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <term.h>
#include <curses.h>

typedef struct lookup_t {
  TermKeySym   sym;
  char         *sequence;
} lookup_t;

static lookup_t *normal_lookup;
static int       normal_lookup_size;

static lookup_t *shift_lookup;
static int       shift_lookup_size;

// Stolen and adapted from libtermkey/driver-ti.c 
static struct {
  const char *funcname;
  TermKeySym sym;
} funcs[] = {
  { "backspace", TERMKEY_SYM_BACKSPACE },
  { "begin",     TERMKEY_SYM_BEGIN     },
  { "beg",       TERMKEY_SYM_BEGIN     },
//{ "btab",      TERMKEY_SYM_TAB, TERMKEY_KEYMOD_SHIFT }
  { "cancel",    TERMKEY_SYM_CANCEL    },
  { "clear",     TERMKEY_SYM_CLEAR     },
  { "close",     TERMKEY_SYM_CLOSE     },
  { "command",   TERMKEY_SYM_COMMAND   },
  { "copy",      TERMKEY_SYM_COPY      },
  { "dc",        TERMKEY_SYM_DELETE    },
  { "down",      TERMKEY_SYM_DOWN      },
  { "end",       TERMKEY_SYM_END       },
  { "enter",     TERMKEY_SYM_ENTER     },
  { "exit",      TERMKEY_SYM_EXIT      },
  { "find",      TERMKEY_SYM_FIND      },
  { "help",      TERMKEY_SYM_HELP      },
  { "home",      TERMKEY_SYM_HOME      },
  { "ic",        TERMKEY_SYM_INSERT    },
  { "left",      TERMKEY_SYM_LEFT      },
  { "mark",      TERMKEY_SYM_MARK      },
  { "message",   TERMKEY_SYM_MESSAGE   },
  { "mouse",     TERMKEY_SYM_NONE      },
  { "move",      TERMKEY_SYM_MOVE      },
  { "next",      TERMKEY_SYM_PAGEDOWN  },
  { "npage",     TERMKEY_SYM_PAGEDOWN  },
  { "open",      TERMKEY_SYM_OPEN      },
  { "options",   TERMKEY_SYM_OPTIONS   },
  { "ppage",     TERMKEY_SYM_PAGEUP    },
  { "previous",  TERMKEY_SYM_PAGEUP    },
  { "print",     TERMKEY_SYM_PRINT     },
  { "redo",      TERMKEY_SYM_REDO      },
  { "reference", TERMKEY_SYM_REFERENCE },
  { "refresh",   TERMKEY_SYM_REFRESH   },
  { "replace",   TERMKEY_SYM_REPLACE   },
  { "restart",   TERMKEY_SYM_RESTART   },
  { "resume",    TERMKEY_SYM_RESUME    },
  { "right",     TERMKEY_SYM_RIGHT     },
  { "save",      TERMKEY_SYM_SAVE      },
  { "select",    TERMKEY_SYM_SELECT    },
  { "suspend",   TERMKEY_SYM_SUSPEND   },
  { "undo",      TERMKEY_SYM_UNDO      },
  { "up",        TERMKEY_SYM_UP        },
  { NULL }
};

// Stolen and adapted from libtermkey/driver-ti.c 
static
int funcname2keysym(const char *funcname, TermKeySym *symp)
{
   int start = 0;
   int end   = sizeof(funcs)/sizeof(funcs[0]);

   while (1) {
      int i   = (start+end) / 2;
      int cmp = strcmp(funcname, funcs[i].funcname);

      if (cmp == 0) {
         *symp = funcs[i].sym;
         return 1;
      }
      else if (end == start + 1)
         // That was our last choice and it wasn't it - not found
         break;
      else if (cmp > 0)
         start = i;
      else
         end = i;
   }

   return 0;
}

static
char* get_sequence_for_sym(lookup_t *table, int size, TermKeySym sym) {
   for (int i=0; i < size; ++i)
      if (table[i].sym == sym)
         return table[i].sequence;
   
   return NULL;
}
 
static 
void addKeySym(TermKeySym sym, char *sequence) {
   normal_lookup[normal_lookup_size].sym      = sym;
   normal_lookup[normal_lookup_size].sequence = sequence;
   ++normal_lookup_size;
}

// Stolen and adapted from libtermkey/driver-ti.c 
int load_terminfo()
{
   int i;
   TermKeySym sym;
   normal_lookup = malloc(sizeof(funcs)/sizeof(funcs[0]) * sizeof(lookup_t));
   normal_lookup_size = 0;
   shift_lookup  = malloc(sizeof(funcs)/sizeof(funcs[0]) * sizeof(lookup_t));
   shift_lookup_size = 0;

   if (setupterm(NULL, 1, NULL) != OK)
      return 0;

   for (i = 0; strfnames[i]; i++)
   {
      // Only care about the key_* constants
      const char *name = strfnames[i];
      if (strncmp(name, "key_", 4) != 0)
         continue;
      if (strcmp(name + 4, "mouse") == 0)
         continue;

      char *value = tigetstr(strnames[i]);
      if (value == 0)
         continue;

      if(funcname2keysym(name + 4, &sym)) {
         if(sym == TERMKEY_SYM_NONE)
            continue;

         normal_lookup[normal_lookup_size].sym = sym;
         normal_lookup[normal_lookup_size].sequence = value;
         ++normal_lookup_size;
      }
      else if (funcname2keysym(name + 5, &sym)) {
         if(sym == TERMKEY_SYM_NONE)
            continue;

         shift_lookup[shift_lookup_size].sym = sym;
         shift_lookup[shift_lookup_size].sequence = value;
         ++shift_lookup_size;
      }
   }

   addKeySym(TERMKEY_SYM_ENTER,     "\x0D");
   addKeySym(TERMKEY_SYM_DEL,       "\x7F");
   addKeySym(TERMKEY_SYM_ESCAPE,    "\x1B");
   // CTRL+Space

   normal_lookup = realloc(normal_lookup, normal_lookup_size * sizeof(lookup_t));
   shift_lookup  = realloc(shift_lookup,  shift_lookup_size * sizeof(lookup_t));

   return 1;
}

void unload_terminfo() {
   free(normal_lookup);
   free(shift_lookup);
}

TermKeyKey* parse_key_new(char *def) {
   TermKeyKey *key = malloc(sizeof(*key));

   if (! parse_key(def, key)) {
      free(key);
      return NULL;
   }

   return key;
}

int parse_key(char *def, TermKeyKey *key) {
   const char *last_char = termkey_strpkey(tk, def, key, 0);
   return (*last_char == 0);
}

char *format_key(TermKeyKey *key) {
   static char buf[42];
   termkey_strfkey(tk, buf, 42, key, 0);
   return buf;
}

static inline __attribute__((always_inline))
char get_byte_for_mod(int modifiers) {
   /*  For KEYSYM and Fn-keys
    *    base        = 49
    *    Shift       =  1
    *    Alt         =  2
    *    Cntrl       =  4  */

   return 49
      + (1 * (!! (modifiers & TERMKEY_KEYMOD_SHIFT)))
      + (2 * (!! (modifiers & TERMKEY_KEYMOD_ALT)))
      + (4 * (!! (modifiers & TERMKEY_KEYMOD_CTRL)));
}

char *get_key_code(TermKeyKey *key) {
   static char buf[8];

   if (key->type == TERMKEY_TYPE_KEYSYM) {
      if (key->modifiers == 0)
         return get_sequence_for_sym(normal_lookup, normal_lookup_size, key->code.sym);

      char c;
      #define case break; case
      switch ((int) key->code.sym) {
         case TERMKEY_SYM_INSERT:    c = '2';
         case TERMKEY_SYM_DELETE:    c = '3';
         case TERMKEY_SYM_PAGEUP:    c = '5';
         case TERMKEY_SYM_PAGEDOWN:  c = '6';
         
         case TERMKEY_SYM_UP:        c = 'A';
         case TERMKEY_SYM_DOWN:      c = 'B';
         case TERMKEY_SYM_RIGHT:     c = 'C';
         case TERMKEY_SYM_LEFT:      c = 'D';
         case TERMKEY_SYM_END:       c = 'F';
         case TERMKEY_SYM_HOME:      c = 'H';

         break;
         default: return NULL;
      }
      #undef case

      char mod = get_byte_for_mod(key->modifiers);

      buf[0] = 27;
      buf[1] = 91;
      if (c <= '6') {
         buf[2] = c;
         buf[3] = 59;
         buf[4] = mod;
         buf[5] = 126;
      }
      else {
         buf[2] = 49;
         buf[3] = 59;
         buf[4] = mod;
         buf[5] = c;
      }
      buf[6] = 0;

      /*   A-Insert: 27  91  50  59  51 126
       *   A-Delete: 27  91  51  59  51 126
       *   A-PageUp: 27  91  53  59  51 126
       * A-PageDown: 27  91  54  59  51 126
       *
       *       A-Up: 27  91  49  59  51  65
       *     A-Down: 27  91  49  59  51  66
       *    A-Right: 27  91  49  59  51  67
       *     A-Left: 27  91  49  59  51  68
       *      A-End: 27  91  49  59  51  70
       *     A-Home: 27  91  49  59  51  72 */
   }
   else if (key->type == TERMKEY_TYPE_FUNCTION) {
      int n = key->code.number;

      if (n > 12)
         return NULL;

      buf[0] = 27;

      if (key->modifiers == 0) {
         /*   F1:  27  79  80  0  0
          *   F2:  27  79  81  0  0
          *   F3:  27  79  82  0  0
          *   F4:  27  79  83  0  0
          *   F5:  27  91  49  53 126
          *   F6:  27  91  49  55 126
          *   F7:  27  91  49  56 126
          *   F8:  27  91  49  57 126
          *   F9:  27  91  50  48 126
          *  F10:  27  91  50  49 126
          *  F11:  27  91  50  51 126
          *  F12:  27  91  50  52 126   */

         if (n <= 4) {
            buf[1] = 79;
            buf[2] = 80 + n - 1;
            buf[3] = 0;
         }
         else {
            buf[1] = 91;
            if (n <= 8) {
               buf[2] = 49;
               buf[3] = (char[]){ 53, 55, 56, 57 }[n - 5];
            } else {
               buf[2] = 50;
               buf[3] = (char[]){ 48, 49, 51, 52 }[n - 9];
            }
            buf[4] = 126;
            buf[5] = 0;
         }
      }
      else {
         char mod = get_byte_for_mod(key->modifiers);

         /*     S-F1:  27  91  49  59  50  80   0
          *     S-F2:  27  91  49  59  50  81   0
          *     S-F3:  27  91  49  59  50  82   0
          *     S-F4:  27  91  49  59  50  83   0
          *     S-F5:  27  91  49  53  59  50 126
          *     S-F6:  27  91  49  55  59  50 126
          *     S-F7:  27  91  49  56  59  50 126
          *     S-F8:  27  91  49  57  59  50 126
          *     S-F9:  27  91  50  48  59  50 126
          *    S-F10:  27  91  50  49  59  50 126
          *    S-F11:  27  91  50  51  59  50 126
          *    S-F12:  27  91  50  52  59  50 126 */

         /*     C-F1:  27  91  49  59  53  80   0 
          *     C-F2:  27  91  49  59  53  81   0 
          *     C-F3:  27  91  49  59  53  82   0 
          *     C-F4:  27  91  49  59  53  83   0 
          *     C-F5:  27  91  49  53  59  53 126 
          *     C-F6:  27  91  49  55  59  53 126 
          *     C-F7:  27  91  49  56  59  53 126 
          *     C-F8:  27  91  49  57  59  53 126 
          *     C-F9:  27  91  50  48  59  53 126 
          *    C-F10:  27  91  50  49  59  53 126 
          *    C-F11:  27  91  50  51  59  53 126 
          *    C-F12:  27  91  50  52  59  53 126 */

         /*   C-S-F1:  27  91  49  59  54  80   0
          *   C-S-F2:  27  91  49  59  54  81   0
          *   C-S-F3:  27  91  49  59  54  82   0
          *   C-S-F4:  27  91  49  59  54  83   0
          *   C-S-F5:  27  91  49  53  59  54 126
          *   C-S-F6:  27  91  49  55  59  54 126
          *   C-S-F7:  27  91  49  56  59  54 126
          *   C-S-F8:  27  91  49  57  59  54 126
          *   C-S-F9:  27  91  50  48  59  54 126
          *  C-S-F10:  27  91  50  49  59  54 126
          *  C-S-F11:  27  91  50  51  59  54 126
          *  C-S-F12:  27  91  50  52  59  54 126 */

         buf[1] = 91;
         buf[2] = (n <= 8 ? 49 : 50);
         buf[3] = (char[]) { 59, 59, 59, 59, 53, 55, 56, 57, 48, 49, 51, 52 } [n - 1];

         if (n <= 4) {
            buf[4] = mod;
            buf[5] = 80 + n - 1;
            buf[6] = 0;
         }
         else {
            buf[4] = 59;
            buf[5] = mod;
            buf[6] = 126;
            buf[7] = 0;
         }
      }
   }
   else if (key->type == TERMKEY_TYPE_UNICODE) {
      char c = key->code.codepoint;

      if (key->modifiers & TERMKEY_KEYMOD_CTRL)
         c -= 0x60;

      if (key->modifiers & TERMKEY_KEYMOD_ALT) {
         buf[0] = 27;
         buf[1] = c;
         buf[2] = 0;
      }
      else {
         buf[0] = c;
         buf[1] = 0;
      }
   }

   return buf;
}
