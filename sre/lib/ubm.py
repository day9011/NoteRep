#!/use/bin/env python2.7
#coding=utf-8

from plugins.CpuInit import InitSRE
import os

all = ["init_ubm", "get_ubm"]


_UBM_ = None

class UBMInit():
    def __init__(self, ubm_dir):
        self.__isInit = False
        dubm = os.path.join(os.path.abspath(ubm_dir), 'final.dubm')
        ie = os.path.join(os.path.abspath(ubm_dir), 'final.ie')
        fubm = os.path.join(os.path.abspath(ubm_dir), 'final.ubm')
        plda = os.path.join(os.path.abspath(ubm_dir), 'plda')
        self.__mean = os.path.join(os.path.abspath(ubm_dir), 'mean.vec')
        try:
            self.__ubm = InitSRE()
            self.__isInit = self.__ubm.ReadUBMFile(fubm, ie, dubm, plda)
        except:
            self.__isInit = False

    def get_ubm(self):
        if self.__isInit:
            return self.__ubm
        else:
            return self.__isInit

    def get_mean(self):
        if self.__isInit:
            return self.__mean
        else:
            return self.__isInit


def init_ubm(ubm_dir):
    global _UBM_
    _UBM_ = UBMInit(ubm_dir)

def get_ubm():
    return _UBM_.get_ubm()

def get_mean():
    return _UBM_.get_mean()


