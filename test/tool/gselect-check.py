#!/bin/env python2.7
#coding=utf-8
import sys

def check_gselect(file1, file2):
    arr1 = []
    for line in file1:
        line = line.strip().lstrip('[').rstrip(']').strip()
        items = line.split('\r')
        for item in items:
            t = item.strip().split(' ')
            arr1.append(t)
    arr2 = []
    for line in file2:
        line = line.strip().lstrip('[').rstrip(']').strip()
        arr2.append(line.split(' '))
    for i in range(len(arr1)):
        index = 0
        for t in arr2[i]:
            if not (t in arr1[i]):
                index += 1
                print "no number ", t
        if index > 0:
            print arr1[i]
            print arr2[i]
            print "%d line has %d error" % (i, index)


if __name__=="__main__":
    if (len(sys.argv) > 3):
        print "error argv"
        sys.exit()
    file1 = open(sys.argv[1], 'r')
    file2 = open(sys.argv[2], 'r')
    check_gselect(file1, file2)
    file1.close()
    file2.close()
