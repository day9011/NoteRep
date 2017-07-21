#!/usr/bin/env python2.7

import sys

def count_utt_score(score_file, trials_file):
    scores = open(score_file, 'r').readlines()
    trials = open(trials_file, 'r').readlines()
    if len(scores) != len(trials):
        print "error lines between scores file and trials file"
        exit(1)

    utt_dict = {}
    for i in range(len(scores)):
        predict = trials[i].strip().split(' ')[2]
        c_score = float(scores[i].strip().split(' ')[2])
        uttid = trials[i].strip().split(' ')[1]
        spkid = trials[i].strip().split(' ')[0]
        if uttid in utt_dict:
            utt_dict[uttid][spkid] = c_score
        else:
            utt_dict[uttid] = {}
            utt_dict[uttid][spkid] = c_score
    index = 0
    total = len(utt_dict)
    first = 0
    third = 0
    non_third_list = []
    for (k, d) in utt_dict.items():
        isSpk = False
        items = d.items()
        backitems = [[v[1],v[0]] for v in items]
        backitems.sort(reverse=True)
        judge_list = backitems[:3]
        spkid = k.strip().split('_')[0]
        if judge_list[0][1] == spkid:
            first += 1
            third += 1
        else:
            for it in judge_list:
                if it[1] == spkid:
                    isSpk = True
                    third += 1
                    continue
            if not isSpk:
                non_third_list.append(spkid + '_' + k)
    print "non third list:"
    for item in non_third_list:
        print item
    print "first ratio:%.6f" % (float(first) / float(total))
    print "third ratio:%.6f" % (float(third) / float(total))



if __name__ == "__main__":
    if len(sys.argv) != 3:
        print "Usage: " + sys.argv[0] + " <score_file:string> <trials_file:string>"
        exit(1)
    score_file = sys.argv[1]
    trials_file = sys.argv[2]
    count_utt_score(score_file, trials_file)
