__author__ = 'dinghanyu'
#!/usr/lib/python
#coding=utf-8

import MySQLdb
from utils.server_log import getlogger
from utils.config import get, set_section
import sys
reload(sys)
sys.setdefaultencoding('utf-8')

__all__ = ['get', 'modify']
logger = getlogger()

#news model class
class Mydb:
    def __init__(self):
        set_section("db")
        self.__username = get('username')
        self.__password = get('passwd')
        self.__db = get('dbname')
        self.__port = int(get('port'))
        self.__host = get('host')
        self.__charset = 'utf8'
        self.conn = None
        self.cursor = None
        self.times = 0
        self.connect_db()

    def connect_db(self):
        try:
            self.conn = MySQLdb.connect(host=self.__host, user=self.__username, passwd=self.__password, db=self.__db, port=self.__port, read_default_file='/etc/my.cnf', charset=self.__charset)
            self.cursor = self.conn.cursor(cursorclass=MySQLdb.cursors.DictCursor)
        except Exception, e:
            logger.error(str(e))

    def connect(self):
        if self.times > 4:
            logger.error('connect database failed!')
            self.times = 0
            return -1, 'connect database failed!'
        try:
            if not self.conn.ping():
                self.connect_db()
                self.times += 1
                logger.warn('Reconnect database.....')
        except:
            logger.warn('Reconnect database.....')
            self.connect()

    def modify(self, sql_str, params=None):
        self.connect()
        if sql_str != "":
            try:
                logger.info("DB modify:" + sql_str)
                self.cursor.execute(sql_str, params)
                self.conn.commit()
                return 0, 'update data successfully'
            except Exception, e:
                logger.error(str(e))
                return -10, str(e)
            finally:
                self.cursor.close()
        else:
            return -2, "no sql command"

    def get(self, sql):
        self.connect()
        try:
            logger.info("DB get:" + sql)
            self.cursor.execute(sql)
            raw_records = self.cursor.fetchall()
            return 0, list(raw_records)
        except Exception, e:
            logger.error(str(e))
            return -10, str(e)
        finally:
            self.cursor.close()

    def disconnect_db(self):
        logger.info("disconnect db")
        if self.conn:
            self.cursor = None
            self.conn.close()
            self.conn = None

