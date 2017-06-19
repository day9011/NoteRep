#!/usr/bin/env python2.7
#coding=utf-8

from tornado.web import RequestHandler
import tornado
from lib.db import Mydb
from utils.server_log import getlogger
import json
from utils.post_valid import *
from utils.get_time import *
from utils.config import get as config_get
from utils.config import set_section
import uuid
import os
from lib.ubm import get_ubm
from plugins.CpuCompute import CpuCompute

set_section('sre')

logger = getlogger()

class SreRegister(RequestHandler):
    @tornado.web.asynchronous
    def post(self):
        ret = {'status': 0, 'message': 'OK'}
        db = Mydb()
        try:
            upfilepath = ''
            ivecfile = ''
            upload_path = config_get('wavdir', 'sre')
            if not os.path.exists(upload_path):
                os.makedirs(upload_path)
            logger.info('upload path is ' + upload_path)
            ivec_path = config_get('ivectordir', 'sre')
            validframes = int(config_get('validframes', 'sre'))
            if not os.path.exists(ivec_path):
                os.makedirs(ivec_path)
            logger.info('ivec path is ' + ivec_path)
            info = self.request.protocol + '://' + self.request.host + ', method=' + self.request.method + ', access url=' + self.request.uri
            logger.info(info)
            body = self.request.body_arguments
            Args = [
                    self_argument('UserId', required=True, helpinfo='miss user id')
                    ]
            s, vals = DataIsValid(Args, body)
            if s:
                raise Exception(vals)
            userid = vals['UserId']
            logger.info('userid is ' + userid)
            if not(userid):
                raise Exception('No user id')
            file_metas = self.request.files['UploadFile']
            wav_uuid = uuid.uuid1()
            logger.info('generate uuid:' + str(wav_uuid))
            wavpath = upload_path + '/' + userid
            if not os.path.exists(wavpath):
                os.makedirs(wavpath)
            for meta in file_metas:
                filename = meta['filename']
                upfilepath = wavpath.rstrip('/') + '/' + str(wav_uuid) + '_' + filename
                with open(upfilepath, 'wb') as up:
                    up.write(meta['body'])
                    up.close()
                    break
            ivecfile_path = ivec_path.rstrip('/') + '/' + userid + '/'
            if not os.path.exists(ivecfile_path):
                os.makedirs(ivecfile_path)
            ivecfile = ivecfile_path.rstrip('/') + '/' + str(wav_uuid) + '.ivec'
            ubm = get_ubm()
            sre_compute = CpuCompute()
            #use str function is to avoid string encode error
            if not sre_compute.Compute(ubm, str(upfilepath), str(ivecfile), int(validframes)):
                raise Exception('compute ivector failed')
            res, _ = db.modify('INSERT INTO user (userid, wavpath, ivecpath) values ("%s", "%s", "%s");' % (userid, upfilepath, ivecfile))
            if res != 0:
                raise Exception('insert record into user error')
        except Exception, e:
            logger.error(str(e))
            ret = {'status': -1, 'message': str(e)}
            if os.path.isfile(upfilepath):
                os.remove(upfilepath)
            if os.path.isfile(ivecfile):
                os.remove(ivecfile)
        finally:
            ret = json.dumps(ret)
            self.write(ret)
            self.finish()

    @tornado.web.asynchronous
    def get(self):
        info = self.request.protocol + '://' + self.request.host + ', method=' + self.request.method + ', access url=' + self.request.uri
        logger.info(info)
        self.render('register.html')
