#!/usr/bin/env python2.7

import sys

def call_perform(score_file, trials_file, threshold):
    threshold = float(threshold)
    scores = open(score_file, 'r').readlines()
    trials = open(trials_file, 'r').readlines()
    tp = 0
    fp = 0
    tn = 0
    fn = 0
    if len(scores) != len(trials):
        print "error lines between scores file and trials file"
        exit(1)

    fp_list = []
    min_tp = 10000
    max_fp = -10000
    max_fp_id = ""
    min_tp_id = ""
    for i in range(len(scores)):
        predict = trials[i].strip().split(' ')[2]
        c_score = float(scores[i].strip().split(' ')[2])
        uttid = trials[i].strip().split(' ')[1]
        spkid = trials[i].strip().split(' ')[0]
        if (predict == 'nontarget') and (max_fp < c_score):
            max_fp = c_score
            max_fp_id = spkid + '_' + uttid
        if (predict == 'target') and (min_tp > c_score):
            min_tp = c_score
            min_tp_id = spkid + '_' + uttid
        if (c_score >= threshold) and (predict == 'target'):
            tp += 1
        elif (c_score >= threshold) and (predict == 'nontarget'):
            fp += 1
            fp_list.append({spkid + '_' + uttid: c_score})
        elif (c_score < threshold) and (predict == 'target'):
            fn += 1
        elif (c_score < threshold) and (predict == 'nontarget'):
            tn += 1
        else:
            print "error situation"
            exit(1)
#    print fp_list
    print "max fp value:", max_fp, "spkid_uttid:", max_fp_id
    print "min tp value:", min_tp, "spkid_uttid:", min_tp_id
    return tp, tn, fp, fn, len(scores)



if __name__ == "__main__":
    if len(sys.argv) != 4:
        print "Usage: " + sys.argv[0] + " score_file trials_file threshold"
        exit(1)
    score_file = sys.argv[1]
    trials_file = sys.argv[2]
    threshold = float(sys.argv[3])
    TP, TN, FP, FN, TOTAL = call_perform(score_file, trials_file, threshold)
    if (TP + TN + FP + FN) != TOTAL:
        print "Maybe statistics error"
    print "TP:", TP, ", TN:", TN, ", FP:", FP, ", FN:", FN, ", TOTAL", TOTAL
    print "false positive er:%.6f" % (float(FP) / float(TP + FP))
    print "false negative er:%.6f" % (float(FN) / float(TP + FN))

