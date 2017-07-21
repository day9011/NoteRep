#!/bin/bash

set -e
set -x

if [ $# -ne 3 ]; then
	echo "Usage: $0 <string:wav_data_dir> <string:outdata_dir> <string:duration>"
	exit 1;
fi

wav_data_dir=$1
outdata_dir=$2
duration=$3

[ -d ${outdata_dir} ] || mkdir -p ${outdata_dir}

for x in `find ${wav_data_dir} -type f | grep -E "*.wav$"`
do
	dir_name=`dirname ${x}`
	spkid=${dir_name##*/}
	filename=`basename ${x}`
	sox ${x} ${outdata_dir}/${spkid}_${filename} trim 0 ${duration}
done
