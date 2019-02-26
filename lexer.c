#include "lexer.h"

#include <ctype.h>
#include <stdlib.h>

static FILE         *lex_in;
static int           lex_is_eof;

#define              LEX_TOKEN_BUF_INC 1024
static char         *lex_token_buf;
static int           lex_token_bufsz;
static int           lex_token_pos;

#define              LEX_BUF_SZ 3
static char          lex_buf[LEX_BUF_SZ];
static int           lex_buf_pos;

#define              LEX_ERROR_BUF_SZ 1024
static int           lex_errno;
static char         *lex_error_buf;

#define LEX_SYM(SYM) \
   ( SYM == LEX_TOKEN_END          ? "END"      :\
   ( SYM == LEX_TOKEN_SINGLE_QUOTE ? "SINGLE"   :\
   ( SYM == LEX_TOKEN_DOUBLE_QUOTE ? "DOUBLE"   :\
   ( SYM == LEX_TOKEN_WORD         ? "WORD"     :\
     "???" ))))

int lex_init(FILE *in) {
   lex_in            = in;
   lex_line          = 1;
   lex_line_pos      = 1;

   lex_is_eof        = 0;

   lex_token_buf     = malloc(LEX_TOKEN_BUF_INC);
   lex_token_pos     = 0;
   lex_token_bufsz   = LEX_TOKEN_BUF_INC;

   lex_errno         = 0;
   lex_error_buf     = NULL;

   lex_buf_pos       = -1;

   if (! lex_token_buf)
      return 0;
   return 1;
}

void lex_destroy() {
   free(lex_token_buf);
   if (lex_error_buf)
      free(lex_error_buf);
}

/*
 * We're handling the line continuation character (\) here,
 * so we don't have to care about it in lex()
 */
static int feed_buf() {
   int c1 = fgetc(lex_in);

   if (c1 == EOF || c1 == 0)
      return EOF;

   if (c1 == '\\') {
      int c2 = fgetc(lex_in);

      if (c2 == '\n') {
         lex_line++;
         lex_line_pos = 1;
         return feed_buf();
      }
      else if (c2 == EOF || c2 == 0) {
         lex_buf[++lex_buf_pos] = c1;
      }
      else {
         lex_buf[++lex_buf_pos] = c2;
         lex_buf[++lex_buf_pos] = c1;
      }
   }
   else {
      lex_buf[++lex_buf_pos] = c1;
   }

   return 1;
}

static int lex_peekc() {
   if (lex_buf_pos == -1) {
      if (feed_buf() == EOF)
         return EOF;
   }

   return lex_buf[lex_buf_pos];
}

static int lex_getc() {
   if (lex_buf_pos == -1) {
      if (feed_buf() == EOF)
         return EOF;
   }

   if (lex_buf[lex_buf_pos] == '\n') {
      lex_line++;
      lex_line_pos = 1;
   } else {
      lex_line_pos++;
   }

   return lex_buf[lex_buf_pos--];
}

static void lex_ungetc(int c) {
   lex_line_pos--;
   lex_buf[++lex_buf_pos] = c;
}

static void token_clear() {
   lex_token_pos = 0;
}

static void token_append(int c) {
   if (lex_token_pos == lex_token_bufsz) {
      lex_token_bufsz += LEX_TOKEN_BUF_INC;
      lex_token_buf = realloc(lex_token_buf, lex_token_bufsz);
   }

   lex_token_buf[lex_token_pos++] = c;
}

static void token_finalize() {
   token_append(0);
}

char* lex_token() {
   return lex_token_buf;
}

static unsigned int read_hex() {
   int  c;
   unsigned int val = 0;

   while (isxdigit((c = lex_getc())))
      val = val * 16 + 
         (c <= '9' ? c - '0'      :
         (c <= 'F' ? c - 'A' + 10 :
                     c - 'a' + 10));
   lex_ungetc(c);
   return val;
}

static unsigned int read_oct() {
   int  c;
   unsigned int val = 0;

   while ((c = lex_getc()) >= '0' && c < '8')
      val = val * 8 + c - '0';
   lex_ungetc(c);
   return val;
}

static unsigned int read_dec() {
   int c;
   unsigned val = 0;

   while (isdigit((c = lex_getc())))
      val = val * 10 + c - '0';
   lex_ungetc(c);
   return val;
}

static int read_double_quote() {
   int  c;

   token_clear();
   while ((c = lex_getc()) != EOF) {
      if (c == '"') {
         token_finalize();
         return LEX_TOKEN_DOUBLE_QUOTE;
      }
      else if (c == '\\') {
         switch ((c = lex_getc())) {
            case 'a':   token_append('\a'); break;
            case 'b':   token_append('\b'); break;
            case 't':   token_append('\t'); break;
            case 'n':   token_append('\n'); break;
            case 'v':   token_append('\v'); break;
            case 'f':   token_append('\f'); break;
            case 'r':   token_append('\r'); break;
            case 'e':   token_append(033);  break;
            case 'x':   token_append(read_hex()); break;
            case '0':   token_append(read_oct()); break;
            case '1': case '2': case '3':
            case '4': case '5': case '6':
            case '7': case '8': case '9':
                        token_append(read_dec()); break;
            default:    token_append(c);
         }
      }
      else
         token_append(c);
   }

   lex_errno = LEX_ERROR_MISSING_DOUBLE_QUOTE;
   return LEX_ERROR;
}

static int read_single_quote() {
   int c;

   token_clear();
   while ((c = lex_getc()) != EOF) {
      if (c == '\'') {
         token_finalize();
         return LEX_TOKEN_SINGLE_QUOTE;
      }
      else if (c == '\\' && lex_peekc() == '\'')
         token_append(lex_getc());
      else
         token_append(c);
   }

   lex_errno = LEX_ERROR_MISSING_SINGLE_QUOTE;
   return LEX_ERROR;
}

static void consume_comment() {
   int c;

   while ((c = lex_getc()) != EOF) {
      if (c == '\n')
         break;
   }
}

static int read_word() {
   int c;

   token_clear();
   while ((c = lex_getc()) != EOF) {
      if (c == '\\' && lex_peekc() != EOF) {
         token_append(c);
         token_append(lex_getc());
      }
      else if (c == '\'' || c == '"' || c == ';' || isspace(c)) {
         lex_ungetc(c);
         break;
      }
      else {
         token_append(c);
      }
   }
   token_finalize();

   return LEX_TOKEN_WORD;
}

int lex() {
   int c;

   while ((c = lex_getc()) != EOF) {
      switch (c) {
         case '#':   consume_comment();
                     break;
         case '"':   return read_double_quote();
         case '\'':  return read_single_quote();
         case ';':
         case '\n':  return LEX_TOKEN_END;
         case ' ':
         case '\t':  break;
         default:    lex_ungetc(c);
                     return read_word();
      }
   }

   lex_is_eof = 1;
   return EOF;
}

int lex_eof() {
   return lex_is_eof;
}

char *lex_error() {
   if (! lex_error_buf)
      lex_error_buf = malloc(LEX_ERROR_BUF_SZ);

   int n = sprintf(lex_error_buf, "%d:%d: ", lex_line, lex_line_pos);

   #define case break; case
   switch (lex_errno) {
   case 0:
      sprintf(lex_error_buf + n, "all good");

   case LEX_ERROR_MISSING_SINGLE_QUOTE:
      sprintf(lex_error_buf + n, "unterminated single quote");

   case LEX_ERROR_MISSING_DOUBLE_QUOTE:
      sprintf(lex_error_buf + n, "unterminated double quote");

   break;
   default:
      sprintf(lex_error_buf + n, "error in lexer");
   }
   #undef case

   return lex_error_buf;
}
