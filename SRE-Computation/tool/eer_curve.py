#!/usr/bin/env python2.7
#coding=utf-8

import os
import sys
import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator


def draw_eer(trials_file, score_file, out_file):
    trials = open(trials_file, 'r').readlines()
    scores = open(score_file, 'r').readlines()
    if len(trials) != len(scores):
        print "error lines with score and trials"
        exit(1)
    num_score = len(trials)
    target_scores = []
    nontarget_scores = []
    for i in range(num_score):
        if trials[i].strip().split(' ')[2] == 'target':
            target_scores.append(float(eval(scores[i].strip().split(' ')[2])))
        else:
            nontarget_scores.append(float(eval(scores[i].strip().split(' ')[2])))
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
    max_x = max(thresholds)
    min_x = min(thresholds)
    print "eer threshold:", eer_th
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
    plt.plot(thresholds, FA_RS,  '.' , label="FA")
    plt.plot(thresholds, FR_RS, '.', label="FR")
    plt.legend(loc='upper right')
    out_file = out_file.strip('.png' + '.png')
    plt.savefig(out_file)


if __name__ == "__main__":
    if len(sys.argv) != 4:
        print "Usage: " + sys.argv[0] + " <string:trials> <string:score> <string:outfile>"
        exit(1)
    trials_file = sys.argv[1]
    score_file = sys.argv[2]
    out_file = sys.argv[3]
    draw_eer(trials_file, score_file, out_file)
