#!/bin/bash

set -e
set -x


wav_data_dir=$1
outdata_dir=$2
duration=$3

[ -d ${outdata_dir} ] || mkdir -p ${outdata_dir}

for x in `find ${wav_data_dir} -type f | grep -E "*.wav$"`
do
	#dir_name=`dirname ${x}`
	#spkid=${dir_name##*/}
	filename=`basename ${x}`
	#sox ${x} ${outdata_dir}/${spkid}_${filename} trim 0 ${duration}
	sox ${x} ${outdata_dir}/${filename} trim 0 ${duration}
done
