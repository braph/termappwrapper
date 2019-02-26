#ifndef _TERMKEYSTUFF_H
#define _TERMKEYSTUFF_H

#include <termkey.h>

#define TERMKEY_TYPE_TO_FLAG(TYPE) \
   (1 << (TYPE))

extern         TermKey *tk;
int            load_terminfo();
int            parse_key(const char *def, TermKeyKey *key);
TermKeyKey*    parse_key_new(const char *def);
const char*    format_key(TermKeyKey *key);
const char*    get_key_code(TermKeyKey *key);

#if FREE_MEMORY
void           unload_terminfo();
#endif

#endif
