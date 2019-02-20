#!/usr/bin/python3

import re
import sys
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('file')
parser.add_argument('-c', action='store_true')
parser.add_argument('-v', default='conf')
args = parser.parse_args()

words = (
    'mode',
    'bind',
    'repeat',
    'ignore_unmapped',

    'goto',
    'write',
    'mask',
    'key',
    'ignore'
)

def strip_conf(s):
    r = ''
    for l in s.split('\n'):
        l = l.strip()
        l = l.rstrip(';')

        if not l:
            continue

        if l.startswith('#'):
            continue

        l = re.sub(' +', ' ', l)

        for word in words:
            l = l.replace(word, word[0])

        r += '%s;\n' % l
    return r

def cstring(s):
    r = ''
    for l in s.split('\n'):
        if l:
            l = l.replace('\\', '\\\\')
            l = l.replace('"', '\\"')
            r += '"%s"\n' % l
    return r

with open(args.file, 'r') as fh:
    content = fh.read(-1)
    stripped = strip_conf(content)

    if args.c:
        stripped = cstring(stripped)
        stripped = 'const char *%s = \n%s;' % (args.v, stripped)

    print(stripped)
