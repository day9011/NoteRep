__author__ = 'dinghanyu'
#!/usr/bin/python
#coding=utf-8


import os
import tornado.web
import tornado.process
import tornado.httpserver
import tornado.ioloop
import tornado.netutil
import tornado.log
import socket
from tornado.options import define, options
import sys
from utils.config import *
init_config("config/my.conf")
set_section("sre")

num_threads = int(get('numthreads'))

from lib.ubm import *
ubm_dir = get("ubmdir")
init_ubm(ubm_dir)

from url import url



define("port", default = int(get("port")), help="run on specific port", type=int)

settings = {
    'static_path' : os.path.join(os.path.dirname(__file__), "static"),
    'cookie_secret' : "61oETzKXQAGaYdkL5gEmGeJJFuYh7EQnp2XdTP1o/Vo=",
    'template_path':os.path.join(os.path.dirname(__file__), "templates"),
    'xsrf_cookies' : False,
    #'autoreload' : True,
    #'debug' : False,
}

application = tornado.web.Application(
    handlers=url,
    # debug=True,
    **settings
)

if __name__ == "__main__":
    server = tornado.httpserver.HTTPServer(application)
    print "Start listening port:" + str(options.port)
    args = sys.argv
    args.append('--log_file_prefix=' + get("logdir"))
    tornado.options.parse_command_line()
    server.bind(options.port)
    server.start(num_threads)
    tornado.ioloop.IOLoop.current().start()
    # sockets = tornado.netutil.bind_sockets(8888)
    # tornado.process.fork_processes(0)
    # server = tornado.httpserver.HTTPServer(application)
    # server.add_sockets(sockets)
    # tornado.ioloop.IOLoop.current().start()

