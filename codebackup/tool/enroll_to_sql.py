#!/usr/bin/env python2.7
#coding=utf-8

import sys
import os
import MySQLdb

class Mydb:
    def __init__(self):
        self._username = 'day9011'
        self._password = '123456'
        self._db = 'sre'
        self._port = 3306
        self._host = 'localhost'
        self._charset = 'utf8'
        self._conn = None
        self._cursor = None

    def connect_db(self):
        try:
            self._conn = MySQLdb.connect(host=self._host, user=self._username, passwd=self._password,
                                         db=self._db, port=self._port, read_default_file='/etc/my.cnf', charset=self._charset)
            self._cursor = self._conn.cursor(cursorclass=MySQLdb.cursors.DictCursor)
        except Exception, e:
            print "connect db:" + str(e)

    def connect(self):
        try:
            if self._conn == None:
                self.connect_db()
            elif not self._conn.ping():
                self.connect_db()
        except:
            print "error connect db"
            exit(1)

    def modify(self, sql_command, params=None):
        try:
            if sql_command != "":
                print sql_command
                self.connect()
                self._cursor.execute(sql_command, params)
                self._conn.commit()
                return 0, "update data successfully"
        except Exception, e:
            print str(e)
            return -1, "error"

    def get(self, sql_command):
        try:
            if sql_command != "":
                print sql_command
                self.connect()
                self._cursor.execute(sql_command)
                raw_records = self._cursor.fetchall()
                return 0, list(raw_records)
        except Exception, e:
            print str(e)
            return -1, "error"


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print "Usage: " + sys.argv[0] + " <string:ivectorsdir>"
        exit(1)
    db = Mydb()
    ivectors_dir = sys.argv[1]
    if not os.path.exists(ivectors_dir):
        print "ivectors dir is not exists"
        exit(1)
    speakername = os.listdir(ivectors_dir)
    for speaker in speakername:
        ivector = ivectors_dir.rstrip('/') + '/' + speaker.rstrip('/') + '/' + speaker + '_enroll.ivec'
        if not os.path.exists(ivector):
            print "cannot find " + speaker + "'s ivector"
            continue
        ivector = os.path.abspath(ivector)
        res, userinfo = db.get('SELECT * FROM user WHERE userid="%s"' % (speaker))
        if len(userinfo) > 0:
            print "the user was enrolled, update enroll voice with new voice"
            sql_command = 'UPDATE user SET ivecpath="%s" WHERE userid="%s";' % (ivector, speaker)
        else:
            sql_command = 'INSERT INTO user (userid, wavpath, ivecpath) VALUES ("%s", "%s", "%s");' % (speaker, "insert from ivec.txt", ivector)
        res, info = db.modify(sql_command)
        if res != 0:
            print info
            exit(1)
