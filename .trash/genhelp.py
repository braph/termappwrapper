#!/usr/bin/python3

import re
import sys
import glob

def extract_help(file):
    result = ''
    with open(file, 'r') as fh:
        it = iter(fh)

        for line in it:
            if '!DOC!' in line:
                break

        for line in it:
            if '*/' in line:
                return result

            result += re.sub('^ *\* ?', '', line)

def gen_help():
    for f in glob.glob('cmd_*.c'):
        helptext = extract_help(f)
        print(helptext)


gen_help()

# h = extract_help(sys.argv[1])
# print(h)

