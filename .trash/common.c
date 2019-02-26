#include "common.h"

int strprefix(const char *string, const char *prefix) {
   return (strstr(string, prefix) == string);
}
