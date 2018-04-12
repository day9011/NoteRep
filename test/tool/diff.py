#!/usr/bin/env python2.7
#coding=utf-8
import sys

if __name__ == '__main__':
    print sys.argv[1], sys.argv[2]
    file1 = open(sys.argv[1], 'r')
    file2 = open(sys.argv[2], 'r')
    text1 = file1.read().strip().split(' ')
    text2 = file2.read().strip().split(' ')
    j = 0
    diff_position = []
    for i in range(len(text1)):
        if text1[i] != text2[i]:
            diff_position.append(i)
            j += 1
    print 'position:', diff_position
    print 'num:', j
