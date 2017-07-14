#!/usr/bin/env python2.7

import sys
import copy

def filter_fn(fn_file, public_file):
    del_file_list = set()
    with open(fn_file, 'r') as fn:
        for line in fn:
            del_id = line.strip().split(' ')[0].split('_')[2]
            del_file_list.add(del_id)
    with open(public_file, 'r') as public_log, open('filter_fn_public.log', 'w') as fw:
        for line in public_log:
            line_id = line.split(';;')[0]
            if not line_id in del_file_list:
                fw.write(line)


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print "Usage: " + sys.argv[0] + " <string:fn_file> <string:public.log>"
        exit(1)
    fn_file = sys.argv[1]
    public_file = sys.argv[2]
    filter_fn(fn_file, public_file)
