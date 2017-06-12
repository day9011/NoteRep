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

set_section("sre")

logger = getlogger()

class SreRegister(RequestHandler):
    @tornado.web.asynchronous
    def post(self):
        ret = json.dumps({'status': 0, 'message': 'OK'})
        db = Mydb()
        try:
            upload_path = config_get("wavdir", "sre")
            if not os.path.exists(upload_path):
                os.makedirs(upload_path)
            logger.info("upload path is " + upload_path)
            info = self.request.protocol + "://" + self.request.host + ", method=" + self.request.method + ", access url=" + self.request.uri
            logger.info(info)
            body = self.request.body_arguments
            Args = [
                    self_argument('userid', required=True, helpinfo="miss user id")
                    ]
            s, vals = DataIsValid(Args, body)
            if s:
                raise Exception(vals)
            userid = vals['userid']
            logger.info("userid is " + userid)
            if not(userid):
                raise Exception("No user id")
            file_metas = self.request.files['file']
            wav_uuid = uuid.uuid1()
            logger.info("generate uuid:" + str(wav_uuid))
            wavpath = upload_path + '/' + userid + "_" + str(wav_uuid)
            for meta in file_metas:
                filename = meta['filename']
                wavpath = wavpath + filename
                with open(wavpath, "wb") as up:
                    up.write(meta['body'])
        except Exception, e:
            logger.error(str(e))
            ret = json.dumps({"status": -1, 'message': str(e)})
        finally:
            self.write(ret)
            self.finish()

    @tornado.web.asynchronous
    def get(self):
        info = self.request.protocol + "://" + self.request.host + ", method=" + self.request.method + ", access url=" + self.request.uri
        logger.info(info)
        self.write('''
        <html>
            <head><title>SRE Register</title></head>
            <body>
                <form action='register' enctype="multipart/form-data" method='post'>
                    <input type='file' name='file'/><br/>
                    <input type='text' name='userid'/><br/>
                    <input type='submit' value='submit'/>
                </form>
            </body>
        </html>
                   ''')
        self.finish()
