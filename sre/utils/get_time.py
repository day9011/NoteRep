#!/usr/bin/env python2.7
# Name: get_time
# Function: get current system time
# Date: 2016-06-09
# Email: day9011@gmail.com
__author__ = 'day9011'

__all__ = ["get_ct"]
import datetime
import time

def get_ct():
    t_format = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    return t_format


if __name__ == "__main__":
    print get_ct()
