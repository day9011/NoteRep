#!/usr/bin/env python2.7

import sys

def call_perform(score_file, trials_file, threshold):
    threshold = float(threshold)
    tp = 0
    fp = 0
    tn = 0
    fn = 0

    fp_list = []
    fn_list = []
    min_tp = 10000
    max_fp = -10000
    max_fp_id = ""
    min_tp_id = ""
    pos_scores = 0.0
    pos_num = 0
    total = 0
    with open(score_file, 'r') as scores, open(trials_file, 'r') as trials:
        for score_line in scores:
            total += 1
            trial_line = trials.readline()
            predict = trial_line.strip().split(' ')[2]
            c_score = float(score_line.strip().split(' ')[2])
            uttid = trial_line.strip().split(' ')[1]
            spkid = trial_line.strip().split(' ')[0]
            if (predict == 'target') and (c_score > 0):
                pos_scores += c_score
                pos_num += 1
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
                fn_list.append({spkid + '_' + uttid: c_score})
            elif (c_score < threshold) and (predict == 'nontarget'):
                tn += 1
            else:
                print "error situation"
                exit(1)
        print "FP LIST:"
        for item in fp_list:
            for (k,v) in item.items():
                print k + "    score:" + " " + str(v)
        print "FN LIST:"
        for item in fn_list:
            for (k,v) in item.items():
                print k + "    score:" + " " + str(v)
        print "max fp value:", max_fp, "spkid_uttid:", max_fp_id
        print "min tp value:", min_tp, "spkid_uttid:", min_tp_id
        print "target mean score (> 0):", (float(pos_scores) / float(pos_num))
        return tp, tn, fp, fn, total



if __name__ == "__main__":
    if len(sys.argv) != 4:
        print "Usage: " + sys.argv[0] + " <string:score_file> <string:trials_file> <string:threshold>"
        exit(1)
    score_file = sys.argv[1]
    trials_file = sys.argv[2]
    threshold = float(sys.argv[3])
    TP, TN, FP, FN, TOTAL = call_perform(score_file, trials_file, threshold)
    if (TP + TN + FP + FN) != TOTAL:
        print "Maybe statistics error"
    print "TP:", TP, ", TN:", TN, ", FP:", FP, ", FN:", FN, ", TOTAL", TOTAL
    print "FPR:%.6f" % (float(FP) / float(FP + TN))
    print "TPR:%.6f" % (float(TP) / float(TP + FN))
    print "True precision:%.6f" % (float(TP) / float(TP + FP))

