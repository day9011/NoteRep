#!/usr/bin/env python2.7
#coding=utf-8

import sys
from CpuCompute import CpuCompute
from CpuInit import InitSRE

if __name__ == "__main__":
    if len(sys.argv) != 8:
        exit(1)
    wav_file = sys.argv[1]
    full_ubm = sys.argv[2]
    my_ie = sys.argv[3]
    gmm = sys.argv[4]
    plda = sys.argv[5]
    ivector_path = sys.argv[6]
    valid_frames = int(sys.argv[7])
    ubm = InitSRE()
    compute = CpuCompute()
    ubm.ReadUBMFile(full_ubm, my_ie, gmm, plda)
    if compute.Compute(ubm, wav_file, ivector_path, valid_frames):
        print "finish compute ivector"
