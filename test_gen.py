#!/usr/bin/env python3

import sys
import random

def rand_unicode():
    u = 0xd800
    while 0xd800 <= u <= 0xdfff:
        u = random.randint(0, 0x10ffff)
    return chr(u)

def rand_text(n):
    return "".join([rand_unicode() for _ in range(n)])

def write_file(name, bs):
    with open(name, 'wb') as f:
        f.write(bs)

def main(script, n, *argv):
    ss = rand_text(int(n))
    utf8 = ss.encode("utf-8")
    write_file("utf8.txt", utf8)
    utf16be = ss.encode("utf-16be")
    write_file("utf16be.txt", utf16be)
    utf16le = ss.encode("utf-16le")
    write_file("utf16le.txt", utf16le)
    utf32be = ss.encode("utf-32be")
    write_file("utf32be.txt", utf32be)
    utf32le = ss.encode("utf-32le")
    write_file("utf32le.txt", utf32le)

if __name__ == '__main__':
    main(*sys.argv)
