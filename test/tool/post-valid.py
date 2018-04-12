#!/bin/env python2.7
#coding=utf-8
import sys

if __name__ == '__main__':
    if len(sys.argv) > 3:
        print "error number of argv:", len(sys.argv)
        exit(-1)
    file1 = open(sys.argv[1], 'r')
    file2 = open(sys.argv[2], 'r')
    text1 = file1.read().replace(']', '').strip().split('[')
    text2 = file2.read().replace(']', '').strip().split('[')
    arr1 = []
    for item in text1:
        item = item.rstrip("]").strip().split(' ')
        if item != ['']:
            arr1.append(item)
    arr2 = []
    for item in text2:
        item = item.rstrip("]").strip().split(' ')
        if item != ['']:
            arr2.append(item)
    if len(arr1) != len(arr2):
        print "dims of two post are different"
        exit(-1)
    error_index = []
    err_ratio = 0
    for i in range(len(arr1)):
        if len(arr1[i]) != len(arr2[i]) and len(arr1[i]) % 2 != 0:
            error_index.append(i)
            continue
        dict_temp1 = {}
        dict_temp2 = {}
        for j in range(len(arr1[i]) / 2):
            dict_temp1[arr1[i][2 * j]] = arr1[i][2 * j + 1]
            dict_temp2[arr2[i][2 * j]] = arr2[i][2 * j + 1]
        sum_temp = 0
        for (k, v) in dict_temp1.items():
            sum_temp += abs(float(dict_temp2[k]) - float(v))
        err_ratio += float(sum_temp) / len(dict_temp1)
    err_ratio = err_ratio / len(arr1) * 100
    print error_index
    print "error ratio:", err_ratio, "%"
