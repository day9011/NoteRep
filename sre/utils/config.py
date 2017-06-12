#!/usr/bin/env python2.7
#coding=utf-8

import ConfigParser
import sys

_CR_ = None

class ConfigRead():

    def __init__(self, filename):
        self._cp = ConfigParser.SafeConfigParser()
        self._cp.read(filename)
        self._section = self._cp.sections()[0]

    def set_section(self, section):
        if self._cp.has_section(section):
            self._section = section
        else:
            return false

    def get(self, key, section=None):
        if section == None:
            section = self._section
        if self._cp.has_option(section, key):
            return self._cp.get(section, key)
        else:
            return False

    def sections(self):
        return self._cp.sections();


def init_config(config_path):
    global _CR_
    _CR_ = ConfigRead(config_path)

def set_section(section):
    return _CR_.set_section(section)

def get(key, section=None):
    return _CR_.get(key, section)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print "Usage: config.py conf"
        exit(1)
    conf_file = sys.argv[1]
    init_config(conf_file)
    print get("logdir", "sre")
