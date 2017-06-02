#!/usr/bin/env python2.7

import os
import sys

if __name__ == "__main__":
    if len(sys.argv) > 2:
        print "Usage: " + sys.argv[0] + " resultfile"
    res_file = open(sys.argv[1], 'r')
    count = 0
    total = 0
    error_list = []
    for line in res_file.readlines():
        if line.strip():
            total += 1
            res = line.strip().split(' ')
            speaker = res[0].split('/')[-1:][0].split('_')[0]
            label = res[-1]
            if label == speaker:
                count += 1
            else:
                error_label = {}
                error_label[speaker] = label
                error_id = {}
                error_id[total] = error_label
                error_list.append(error_id)
    print "er: %.6f" % (float(count) / float(total))
    print error_list
