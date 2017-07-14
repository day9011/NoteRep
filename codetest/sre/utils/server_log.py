#!/usr/bin/env python2.7
#coding=utf-8
import logging
import logging.handlers
import os

from config import get as config_get

def getlogger():
    log_path = config_get("logdir")
    if not log_path:
        print "log path can't find in config file'"
        exit(1)

    if not os.path.exists(os.path.dirname(log_path)):
        os.makedirs(os.path.dirname(log_path))
    logger = logging.getLogger()
    logger.setLevel(logging.DEBUG)

    if not logger.handlers:
        handler = logging.handlers.TimedRotatingFileHandler(log_path, 'D')
        # fmt = logging.Formatter(“%(asctime)s – %(pathname)s – %(filename)s – %(funcName)s – %(lineno)s – %(levelname)s – %(message)s”, “%Y-%m-%d %H:%M:%S”)
        fmt = logging.Formatter("%(asctime)s – %(pathname)s – %(filename)s – %(funcName)s – %(lineno)s – %(levelname)s – %(message)s")
        handler.setFormatter(fmt)
        logger.addHandler(handler)
    return logger

