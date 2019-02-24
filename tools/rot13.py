#!/usr/bin/python3

def rot13(start, end):
    start = ord(start)
    end = ord(end)

    for i in range(start, end + 1):
        r = i + 13
        if (r > end):
            r -= (end - start + 1)
        print('bind', chr(i), 'key', chr(r))

rot13('a', 'z')
rot13('A', 'Z')
