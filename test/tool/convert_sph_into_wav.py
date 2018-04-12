#!/usr/bin/env python2.7

import sys
import os

sph_command = ""

def ConvertWavFile(filepath):
    if os.path.isdir(filepath):
        for item in os.listdir(filepath):
            ConvertWavFile(filepath + '/' + item)
    else:
        os.system(sph_command + ' -f wav -p -c 1 ' + filepath + ' ' + filepath + '.converted')
        os.rename(filepath + '.converted', filepath)

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print "Usage: make_train_data.py target_dir sph_command_dir"
        exit(1)
    target_dir = sys.argv[1]
    sph_command_dir = sys.argv[2].rstrip('/') + '/'
    sph_command = sph_command_dir + 'sph2pipe'
    if os.path.isdir(target_dir):
        ConvertWavFile(target_dir)
    else:
        print "target dir is not exist"
        exit(1)
