#!/usr/bin/env python2.7
#coding=utf-8

import os
import sys
import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator

def get_coord(target_scores, nontarget_scores):
    target_scores.sort()
    nontarget_scores.sort(reverse=True)
    target_size = len(target_scores)
    nontarget_size = len(nontarget_scores)
    thresholds = []
    FR_RS = []
    FA_RS = []
    eer_th = 0
    for i in range(0, target_size, 1):
        FR = i
        FA = 0
        threshold = target_scores[i]
        for score in nontarget_scores:
            if float(score) < float(threshold):
                break
            else:
                FA += 1
        FA_R = float(FA) / float(nontarget_size)
        FR_R = float(FR) / float(target_size)
        thresholds.append(threshold)
        if abs(FA_R - FR_R) < 0.01:
            eer_th = threshold
        FR_RS.append(FR_R)
        FA_RS.append(FA_R)
        if FA_R <= 0:
            break
    return eer_th, thresholds, FA_RS, FR_RS


def draw_eer(trials_file, score_file1, score_file2, out_file):
    trials = open(trials_file, 'r').readlines()
    scores1 = open(score_file1, 'r').readlines()
    scores2 = open(score_file2, 'r').readlines()
    if len(trials) != len(scores1):
        print "error lines with score1 and trials"
        exit(1)
    if len(trials) != len(scores2):
        print "error lines with score2 and trials"
        exit(1)
    num_score = len(trials)
    target_scores1 = []
    nontarget_scores1 = []
    for i in range(num_score):
        if trials[i].strip().split(' ')[2] == 'target':
            target_scores1.append(float(eval(scores1[i].strip().split(' ')[2])))
        else:
            nontarget_scores1.append(float(eval(scores1[i].strip().split(' ')[2])))
    target_scores2 = []
    nontarget_scores2 = []
    for i in range(num_score):
        if trials[i].strip().split(' ')[2] == 'target':
            target_scores2.append(float(eval(scores2[i].strip().split(' ')[2])))
        else:
            nontarget_scores2.append(float(eval(scores2[i].strip().split(' ')[2])))
    eer_th1, thresholds1, FA_RS1, FR_RS1 = get_coord(target_scores1, nontarget_scores1)
    eer_th2, thresholds2, FA_RS2, FR_RS2 = get_coord(target_scores2, nontarget_scores2)
    max_x1 = max(thresholds1)
    min_x1 = min(thresholds1)
    max_x2 = max(thresholds2)
    min_x2 = min(thresholds2)
    max_x = max(max_x1, max_x2)
    min_x = min(min_x1, min_x2)
    print "mfcc eer threshold:", eer_th1
    print "mfcc+plp eer threshold:", eer_th2
    print("minx, maxx", min_x, max_x)
    fig, ax = plt.subplots()
    plt.title("EER curve")
    plt.xlabel('threshhold')
    plt.ylabel('FA/FR')
    yminorLocator = MultipleLocator(0.1)
    ax.yaxis.set_minor_locator(yminorLocator)
    ax.set_ylim([0, 1])
    ax.set_xlim([min_x, max_x])
    ax.yaxis.grid(yminorLocator)
    ax.xaxis.grid(True, which='major')
    ax.yaxis.grid(True, which='minor')
    plt.plot(thresholds1, FA_RS1, linewidth = '2', label = "MFCC_FA", color='C1', linestyle='-', marker='.')
    plt.plot(thresholds1, FR_RS1, linewidth = '2', label = "MFCC_FR", color='C2', linestyle='-', marker='.')
    plt.plot(thresholds2, FA_RS2, linewidth = '1', label = "MFCC+PLP_FA", color='C3', linestyle='-', marker='o', markersize = '1')
    plt.plot(thresholds2, FR_RS2, linewidth = '1', label = "MFCC+PLP_FR", color='C4', linestyle='-', marker='o', markersize = '1')
    #  plt.plot(thresholds1, FA_RS1, 'b', '.', label="MFCC_FA")
    #  plt.plot(thresholds1, FR_RS1, 'b', '.', label="MFCC_FR")
    #  plt.plot(thresholds2, FA_RS2, 'r', '.', label="MFCC+PLP_FA")
    #  plt.plot(thresholds2, FR_RS2, 'r', '.', label="MFCC+PLP_FR")
    plt.legend(loc='upper right')
    out_file = out_file.rstrip(r'png') + r'png'
    plt.savefig(out_file)


if __name__ == "__main__":
    if len(sys.argv) != 5:
        print "Usage: " + sys.argv[0] + " <string:trials> <string:mfcc_score> <string:plp_score> <string:outfile>"
        exit(1)
    trials_file = sys.argv[1]
    mfcc_score_file = sys.argv[2]
    plp_score_file = sys.argv[3]
    out_file = sys.argv[4]
    draw_eer(trials_file, mfcc_score_file, plp_score_file, out_file)
