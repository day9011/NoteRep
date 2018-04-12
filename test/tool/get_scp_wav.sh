#!/bin/bash

set -x
set -e

if [ $# -ne 2 ]; then
	echo "Usage: $0 <string:wav_scp> <string:out_dir>"
	exit 1;
fi

wav_scp=$1
out_dir=$2

[ -d ${out_dir} ] || mkdir -p ${out_dir} || exit 1;

for x in `cat ${wav_scp} | awk -F" " '{print $3}'`
do
	spkid=`dirname ${x}`
	spkid=${spkid##*/}
	[ -d ${out_dir}/${spkid} ] || mkdir -p ${out_dir}/${spkid} || exit 1;
	$(cp ${x} ${out_dir}/${spkid})
done
