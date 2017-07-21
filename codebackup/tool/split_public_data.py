#!/usr/bin/env python2.7
#coding=utf-8

#only for UTF-8 encode

import sys
import os
import shutil

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print "Usage:" + sys.argv[0] + " <string:data_path> <string:data_log> <string:out_path>"
        exit(1)
    public_data_path = sys.argv[1]
    public_log_path = sys.argv[2]
    split_file_path = sys.argv[3]
    file_info = open(public_log_path, "rb").readlines()
    if not os.path.exists(split_file_path):
        os.makedirs(split_file_path)
    print "makdir " + split_file_path + " successfully"
    if not os.path.exists(os.path.join(os.path.abspath(split_file_path), 'train')):
        os.makedirs(os.path.join(os.path.abspath(split_file_path), 'train'))
    print "makdir " + split_file_path + "/train successfully"
    train_set_dir = os.path.join(os.path.abspath(split_file_path), 'train')
    if not os.path.exists(os.path.join(os.path.abspath(split_file_path), 'test')):
        os.makedirs(os.path.join(os.path.abspath(split_file_path), 'test'))
    print "makdir " + split_file_path + "/test successfully"
    test_set_dir = os.path.join(os.path.abspath(split_file_path), 'test')
    #  if not os.path.exists(os.path.join(os.path.abspath(split_file_path), 'imposter')):
    #      os.makedirs(os.path.join(os.path.abspath(split_file_path), 'imposter'))
    #  print "makdir " + split_file_path + "/imposter successfully"
    count_dict = {}
    num_lines = 0
    for line in file_info:
        record = line.split(';;')
        speaker = record[5]
        confidence = int(record[-1])
        if confidence < 75:
            continue
        if record[5] in count_dict.keys():
            count_dict[record[5]] += 1
        else:
            count_dict[record[5]] = 1
        if len(speaker) < 16:
            print line
        filename = record[0]
        text = record[6]
        text = text.replace(' ', '')
        text_space = ''
        for i in range(len(text) / 3):
            text_space += text[i * 3: i * 3 + 3] + ' '
        amr_file_path = os.path.abspath(public_data_path) + '/' + filename + '.amr'
        speaker_path = os.path.join(os.path.abspath(split_file_path), speaker)
        if not os.path.exists(speaker_path):
            os.makedirs(speaker_path)
        shutil.copy(amr_file_path, speaker_path + '/' + filename + '.amr')
        with open(speaker_path + '/' + filename + '.txt', "w") as f:
            f.write(text_space)
        num_lines += 1
    print "handle %d infos" % (num_lines)
    print "finish split file with log file"
    print "slite dataset with speaker utts number"
    for (k, v) in count_dict.items():
        if v < 2:
            shutil.rmtree(os.path.join(os.path.abspath(split_file_path), k))
        elif v == 2:
            shutil.move(os.path.join(os.path.abspath(split_file_path), k), test_set_dir)
        #  elif v == 3:
        #      shutil.move(os.path.join(os.path.abspath(split_file_path), k), os.path.join(os.path.abspath(split_file_path), 'imposter'))
        else:
            shutil.move(os.path.join(os.path.abspath(split_file_path), k), train_set_dir)
    print "finish split"
