#!/usr/bin/env python2.7
#coding=utf-8

import sys
import os

def filter_confidence(public_file, confidence):
    with open(public_file, 'r') as f, open('filter_confidence.log', 'w') as fw:
        for line in f:
            confidence_line = float(line.strip().split(';;')[-1])
            if confidence_line > confidence:
                fw.write(line)


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print "Usage:" + sys.argv[0] + " <string:public_log> <int:confidence>"
        exit(1)
    public_file = sys.argv[1]
    confidence = int(sys.arv[2])
    filter_confidence(public_file, confidence)
