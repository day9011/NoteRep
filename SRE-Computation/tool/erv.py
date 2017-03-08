#!/bin/env python2.7
#coding=utf-8
import sys

def calcu_vector(file1, file2):
    text1 = file1.read().strip()
    text2 = file2.read().strip()
    text1 = text1.lstrip('[').rstrip(']').strip()
    text2 = text2.lstrip('[').rstrip(']').strip()
    arr1 = text1.split(' ')
    arr2 = text2.split(' ')
    total_err = 0
    opposite_sign = 0
    for i in range(len(arr1)):
        if(float(arr1[i]) > 0 and float(arr2[i]) < 0) or (float(arr1[i]) < 0 and float(arr2[i]) > 0):
            opposite_sign += 1
        total_err += (abs(float(arr1[i]) - float(arr2[i])) / abs(float(arr1[i])))
    print "error ratio:", float(total_err) / len(arr1) * 100, "%"
    print "opposite sign num:", opposite_sign

def calcu_feats(file1, file2):
    arr1 = []
    linelen = 0
    for line in file1:
        line = line.strip().lstrip('[').rstrip(']').strip()
        linelen = len(line.split(' '))
        arr1.extend(line.split(' '))
    print "line length:", linelen
    arr2 = []
    for line in file2:
        line = line.strip().lstrip('[').rstrip(']').strip()
        arr2.extend(line.split(' '))
    total_err = 0
    opposite_sign = 0
    sign_position = []
    zero_postion = []
    for i in range(len(arr1)):
        if(float(arr1[i]) > 0 and float(arr2[i]) < 0) or (float(arr1[i]) < 0 and float(arr2[i]) > 0):
            opposite_sign += 1
            sign_position.append({"(" + str(i // linelen) + "," + str(i % linelen) + ")":arr1[i] + " " + arr2[i]})
        if abs(float(arr1[i])) == 0:
            zero_postion.append({"(" + str(i // linelen) + "," + str(i % linelen) + ")":arr1[i] + " " + arr2[i]})
        else:
            total_err += (abs(float(arr1[i]) - float(arr2[i])) / abs(float(arr1[i])))
    print sign_position
    print "error ratio:", (float(total_err) / len(arr1) * 100), "%"
    print "opposite sign num:", opposite_sign

if __name__=="__main__":
    if (len(sys.argv) > 4):
        print "error argv"
        sys.exit()
    file1 = open(sys.argv[1], 'r')
    file2 = open(sys.argv[2], 'r')
    choose = sys.argv[3]
    if choose == "matrix":
        calcu_feats(file1, file2)
    else:
        calcu_vector(file1, file2)
    file1.close()
    file2.close()
