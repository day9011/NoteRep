#!/usr/bin/env python2.7
#coding=utf-8

import sys
import os


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print "Usage: " + sys.argv[0] + " <string:spk_ivector.txt> <string:store_ivec_path>"
        exit(1)
    spk_ivecs_filename = sys.argv[1]
    store_ivec_path = sys.argv[2]
    if not os.path.exists(store_ivec_path):
        print "cannot find ivector store path"
        exit(1)
    spk_ivecs = open(spk_ivecs_filename).readlines()
    for line in spk_ivecs:
        ivec_info = line.strip()
        speaker_name = ivec_info.split('  ')[0].strip()
        ivec = ivec_info.split('  ')[1].strip()
        ivec_path = store_ivec_path.rstrip('/') + '/' + speaker_name
        if os.path.exists(ivec_path):
            if os.path.exists(ivec_path.rstrip('/') + '/' + speaker_name + '_enroll.ivec'):
                print speaker_name + " ivector is exists"
                continue
            else:
                with open((ivec_path.rstrip('/') + '/' + speaker_name + '_enroll.ivec'), 'w') as ivec_write:
                    ivec_write.write(ivec)
                    ivec_write.close()
                    continue
        else:
            os.makedirs(ivec_path)
            with open((ivec_path.rstrip('/') + '/' + speaker_name + '_enroll.ivec'), 'w') as ivec_write:
                ivec_write.write(ivec)
                ivec_write.close()

