#!/bin/bash

set -e
set -x

echo "Split data set into train ,test and valid."

dir=$1
output=$2

echo "creating ${output}/{train, test, valid}"
[ ! -d ${output}/{train,test,valid} ] || mkdir -p ${output}/{train,test,valid} || exit 1;

cd ${dir}
for x in `find . -type f|grep -E "*.wav"`;do
	index=`echo ${x} | awk -F"_" '{print $2}' | awk -F"." '{print $1}'`
	if [ "${index}" -le "7"  ];then
		cp ${x} ${output}/train/
	else
		cp ${x} ${output}/valid/
#		if [ "${index}" -le "8" ];then
#			cp ${x} ${output}/test/
#		else
#			cp ${x} ${output}/valid/
#		fi
	fi
done
