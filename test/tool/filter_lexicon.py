#!/usr/bin/env python2.7
#coding=utf-8

import sys
import os
import re


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print "Usage:", sys.argv[0], " <string:lexicon.txt> <string:outlexicon.txt>"
        exit(1)
    lexicon_file = sys.argv[1]
    lexicon_out = sys.argv[2]
    lexicon = open(lexicon_file, "r").readlines()
    lexicon_out_path = os.path.dirname(lexicon_out)
    if len(lexicon_out_path.strip()) == 0:
        lexicon_out_path = './'
    if not os.path.exists(lexicon_out_path):
        os.makedirs(lexicon_out_path)
    index = 0
    with open(lexicon_out, "w") as lout:
        for line in lexicon:
            text = line[line.strip().find(' ') + 1 :]
            pattern = re.compile(r"[0-9a-zA-Z\s]")
            match = pattern.match(text)
            if match:
                lout.write(line)
            index += 1
    print "total lines:", index



