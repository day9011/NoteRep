# Author: Yi 2016
# @function: 1. detection accuracy with rank and dot threshold
# 
# @Input: score file (e.g., verification/exp/trials/foo), delete_num, threshold, top_k
# 
# @Output: 1. Portion of filtered users
#          2. Corresponding detection accuracy

#!/usr/bin/python
#coding=utf-8
import sys
import random
	
def random_delete_dict(target_dict, delete_num, target_id):
	delete_arr = []
	keys = list(target_dict)
	if delete_num > len(keys) - 1:
		return target_dict
	while len(delete_arr) <= delete_num:
		tmp = int(random.random() * len(keys))
		if tmp not in delete_arr and keys[tmp] != target_id:
			delete_arr.append(tmp)
	
	deleted_num = 0
	for i in delete_arr:
		target_dict.pop(keys[i])
	return target_dict


def cal_rank(file1, delete_num, threshold, top_k):
	usr_dict = dict()
	target_dict = dict()
	for line in file1:
		line = line.split(' ')
		line2 = line[1].split('_')
		if int(line2[3]) == 0:
			continue
		target_id = int(line[0])
		src_id = int(line2[0])	
		score = float(line[-1])
		if usr_dict.has_key(line[1]):
			usr_dict[line[1]].update({target_id: score})
		else:
			usr_dict.update({line[1]:{target_id: score}})
	
	tot_num = 0
	er = 0
	fil_num = 0
	target_value_arr = []
	for test_id in list(usr_dict.keys()):
		src_id = int(test_id.split('_')[0])
		usr_dict[test_id] = random_delete_dict(usr_dict[test_id], delete_num, src_id)
		if usr_dict[test_id][src_id] > threshold:
			val_arr = sorted(usr_dict[test_id].values(), reverse = True);
			if val_arr.index(usr_dict[test_id][src_id]) > top_k:
				er += 1
			target_value_arr.append(usr_dict[test_id][src_id])
			tot_num += 1
		else:
			fil_num += 1
	return tot_num, er, fil_num


if __name__=="__main__":
	if(len(sys.argv) != 5 or float(sys.argv[2]) <= 0):
		print "error argv"
		print "Usage: python compute_rank.py <file, i.e. foo> <delete_num> <threshold> <top_k>"
		sys.exit()
	delete_num = int(sys.argv[2])
	threshold = float(sys.argv[3])
	top_k = int(sys.argv[4])
	er_num = 0
	tot_num = 0
	fil_num = 0
	for i in range(40):
		file1 = open(sys.argv[1], 'r')
		t_tot_num, t_er_num, t_fil_num = cal_rank(file1, delete_num, threshold, top_k)
		er_num += t_er_num
		tot_num += t_tot_num
		fil_num += t_fil_num
		file1.close()
	print "======Testing", 26-delete_num," Candidates======"
	print "threshold: ", threshold, "top_k: ",top_k + 1
	print "Accuracy of target user after filter is:", (1 - er_num*1.0/tot_num) * 100,"%"
	print "Recall rate of target user: ", (tot_num * 1.0)/(tot_num + fil_num) * 100,"%"

