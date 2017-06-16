#!/usr/bin/env python2.7
#coding=utf-8

import sys
from CpuInit import InitSRE
from CpuScorePlda import ScorePLDA

if __name__ == "__main__":
    if len(sys.argv) != 8:
        exit(1)
    full_ubm = sys.argv[1]
    my_ie = sys.argv[2]
    gmm = sys.argv[3]
    plda = sys.argv[4]
    enroll = sys.argv[5]
    test = sys.argv[6]
    mean = sys.argv[7]
    ubm = InitSRE()
    scorePlda = ScorePLDA()
    ubm.ReadUBMFile(full_ubm, my_ie, gmm, plda)
    score = scorePlda.score_plda(ubm.ubm, enroll, test, mean)
    print "score:", score
