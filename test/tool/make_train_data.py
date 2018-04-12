#!/usr/bin/env python2.7

import sys
import os

def RenameWavFile(filepath, count):
    if os.path.isdir(filepath):
        count = 0
        for item in os.listdir(filepath):
            count += 1
            RenameWavFile(filepath + '/' + item, count)
    else:
        lastdirname = filepath.split('/')[-2:-1][0]
        os.rename(filepath, os.path.dirname(filepath) + '/' + lastdirname + '_' + str(count) + '.wav')

if __name__ == '__main__':
    if len(sys.argv) > 2:
        print "Usage: make_train_data.py target_dir"
        exit(1)
    target_dir = sys.argv[1]
    if os.path.isdir(target_dir):
        RenameWavFile(target_dir, 0)
    else:
        print "target dir is not exist"
        exit(1)
