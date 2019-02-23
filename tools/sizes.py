#!/usr/bin/python3

printsz = '''
printf("%%s = %%d\\n", "%s", sizeof(%s));
'''

main = '''
#include <stdio.h>
#include "iwrap.h"

int main() {
    %s
    return 0;
}
'''

structs = (
    'command_call_t',
    'binding_t',
    'context_t',
    'command_opt_t',
    'command_t',
    'keymode_t'
)

print(main % ('\n'.join(map(lambda s: printsz % (s,s), structs))))

