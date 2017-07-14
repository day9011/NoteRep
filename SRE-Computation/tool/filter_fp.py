#!/usr/bin/env python2.7
#coding=utf-8

import sys
import copy

def unit_fp(fp_file, public_file):
    fp_list = open(fp_file, 'r').readlines()
    fp_set_list = []
    for line in fp_list:
        fp_info = line.strip().split('_')
        speaker_id1 = fp_info[0]
        speaker_id2 = fp_info[1].strip().split(' ')[0]
        speaker_list = set([speaker_id1, speaker_id2])
        fp_set_list.append(speaker_list)
    print "fp array length:", len(fp_set_list)
    unit_fp_list = fp_set_list
    #unit fp speaker
    #遍历speaker_list直到没有相关项，有相关项就合并speaker
    while True:
        temp_fp_list = []
        for set_item in unit_fp_list:
            if len(temp_fp_list) < 1:
                temp_fp_list.append(set_item)
                continue
            ischange = False
            for i in range(len(temp_fp_list)):
                if ischange:
                    break
                temp_set = temp_fp_list[i]
                for item in set_item:
                    if item in temp_set:
                        print "set_item:", set_item, "  temp_set", temp_set, "temp_fp_list", temp_fp_list
                        temp_set = temp_set | set_item
                        temp_fp_list[i] = temp_set
                        print "set_item:", set_item, "  temp_set", temp_set, "temp_fp_list", temp_fp_list
                        ischange = True
                        break
            if not ischange:
                temp_fp_list.append(set_item)
        if len(temp_fp_list) == len(unit_fp_list):
            break
        else:
            unit_fp_list = copy.deepcopy(temp_fp_list)
    print unit_fp_list
    print "unit fp list length:", len(unit_fp_list)
    public_log = open(public_file, 'r').readlines()
    temp_log = copy.deepcopy(public_log)
    for item_set in unit_fp_list:
        rm_element = list(item_set)[1:]
        for rm_item in rm_element:
            for line in public_log:
                if rm_item in line:
                    temp_log.remove(line)
        public_log = copy.deepcopy(temp_log)
    with open('filter_fp_public.log', 'w') as fw:
        for line in public_log:
            fw.write(line)


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print "Usage: " + sys.argv[0] + " <string:fp_file> <string:public.log>"
        exit(1)
    fp_file = sys.argv[1]
    public_file = sys.argv[2]
    unit_fp(fp_file, public_file)
