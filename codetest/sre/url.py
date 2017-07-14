__author__ = 'dinghanyu'
#!/usr/lib/python
#coding=utf-8

import sys
reload(sys)
sys.setdefaultencoding('utf-8')

from api import *

url = [
    (r"/register", register.SreRegister),
    (r"/valid", valid.SreValid),
    (r"/recognize", recognize.SreReconize),
]
