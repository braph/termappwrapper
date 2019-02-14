#ifndef _TERMKEYSTUFF_H
#define _TERMKEYSTUFF_H

#include <termkey.h>

extern TermKey *tk;

int            load_terminfo();
int            parse_key(char *def, TermKeyKey *key);
TermKeyKey*    parse_key_new(char *def);
char*          format_key(TermKeyKey *key);
char*          get_key_code(TermKeyKey *key);

#define TERMKEY_TYPE_TO_FLAG(TYPE) \
   (1 << (TYPE))

#if FREE_MEMORY
void           unload_terminfo();
#endif

#endif
