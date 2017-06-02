#!/bin/bash

set -e
set -x

echo "Split data set into train ,test and valid."

dir=$1
output=$2

echo "creating ${output}/{train, test, valid}"
[ -d ${output}/train ] || mkdir -p ${output}/{train,test,valid} || exit 1;

cd ${dir}
for x in `find . -type f|grep -E "*.wav"`;do
	name=`echo ${x} | awk -F"_" '{print $1}'`
	name=${name%/*}
	name=${name##*/}
	echo ${name}
	index=`echo ${x} | awk -F"_" '{print $2}' | awk -F"." '{print $1}'`
	if [ "${index}" -le "7"  ];then
		[ -d ${output}/train/${name} ] || mkdir -p ${output}/train/${name}
		cp ${x} ${output}/train/${name}
	else
		[ -d ${output}/valid/${name} ] || mkdir -p ${output}/valid/${name}
		cp ${x} ${output}/valid/${name}
#		if [ "${index}" -le "8" ];then
#			cp ${x} ${output}/test/
#		else
#			cp ${x} ${output}/valid/
#		fi
	fi
done
