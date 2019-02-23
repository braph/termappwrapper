#!/usr/bin/python3

import re
import sys
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('file')
parser.add_argument('-c', action='store_true')
parser.add_argument('-v', default='conf')
args = parser.parse_args()

words = {
    'mode':             'mo',
    'bind':             'b',
    'repeat':           're',
    'ignore_unmapped':  'ig',

    'goto':             'go',
    'write':            'wr',
    'mask':             'ma',
    'key':              'ke',
    'ignore':           'ig'
}

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

        for search, replace in words.items():
            l = l.replace(search, replace)

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
