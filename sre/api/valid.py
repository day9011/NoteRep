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
from lib.ubm import get_mean
from plugins.CpuCompute import CpuCompute
from plugins.CpuScorePlda import ScorePLDA

set_section('sre')

logger = getlogger()

class SreValid(RequestHandler):
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
                    self_argument('UserId', required=True, helpinfo='miss user id'),
                    self_argument('IsSelf', required=True, helpinfo='miss isself symbol')
                    ]
            s, vals = DataIsValid(Args, body)
            if s:
                raise Exception(vals)
            userid = vals['UserId']
            isself = vals['IsSelf']
            isself_symbol = False
            if isself == 'yes':
                isself_symbol = True
            elif isself != 'no':
                logger.warning('isself param maybe wrong:' + isself)
            logger.info('userid is ' + userid)
            if not(userid):
                raise Exception('No user id')
            res, userinfo = db.get('SELECT * FROM user WHERE userid="%s"' % (userid))
            if res != 0:
                raise Exception('cannot find user: %s information.' % (userid))
            if len(userinfo) > 0:
                latest_info = sorted(userinfo, key=lambda x:int(x['timeid']), reverse=True)[0]
            else:
                raise Exception('%s\'s userinfo is empty' % (userid))
            logger.info('query user %s info:%s' % (userid, str(latest_info)))
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
            sre_score = ScorePLDA()
            mean = get_mean()
            if not mean:
                raise Exception('ubm is not inited')
            #use str function is to avoid string encode error
            score = float(sre_score.score_plda(ubm.ubm, str(latest_info['ivecpath']), str(ivecfile), str(mean)))
            res, _ = db.modify('INSERT INTO valid (userid, wavpath, ivecpath, isself, score) values ("%s", "%s", "%s", %s, %f);' % (userid, upfilepath, ivecfile, isself_symbol, score))
            if res != 0:
                raise Exception('insert record into user error')
            ret['message'] = 'score is %f' % (score)
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
        self.render('valid.html')
