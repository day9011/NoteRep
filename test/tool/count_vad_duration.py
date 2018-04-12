#!/usr/bin/env python2.7
#coding=utf-8

import sys

if __name__ == "__main__":
    if len(sys.argv) > 2:
        print "error number of argv:", len(sys.argv)
        exit(1)
    vad_file = open(sys.argv[1], 'r')
    vad_result = vad_file.read()
    vad_vec = vad_result.strip(' ').lstrip('[').rstrip(']').strip(' ')
    vad_list = vad_vec.split(' ')
    duration_frames = []
    count = 1
    for index in range(len(vad_list)):
        if index == len(vad_list) - 1:
            break
        if vad_list[index + 1] == vad_list[index]:
            count += 1
        else:
            duration_frames.append(count)
            count = 1
    print duration_frames

