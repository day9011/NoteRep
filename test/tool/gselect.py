#!/bin/env python2.7
#coding=utf-8
import sys

if __name__ == '__main__':
    if len(sys.argv) > 2:
        print "error number of argv:", len(sys.argv)
        exit(1)
    print sys.argv[1]
    file1 = open(sys.argv[1], 'r')
    text1 = file1.read().strip().split(";")
    print len(text1) - 1
    index = 0
    for i in range(len(text1) - 1):
        ar = text1[i].split(" ")
        if index != len(ar):
            print ar
            print len(ar)
            index = len(ar)
