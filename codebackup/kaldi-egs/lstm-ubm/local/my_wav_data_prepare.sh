#!/bin/bash

wav_dir=$1
data_set_name=$2
out_set_name=$3

[ -d data/${out_set_name} ] || mkdir -p data/${out_set_name} || exit 1;

echo "clear data/${out_set_name}"
rm -rf data/${out_set_name}/{wav.scp,utt2spk,spk2utt} 2>/dev/null
echo "preparing scps in data/${out_set_name}"
for x in `find ${wav_dir}/${data_set_name} -type f | grep -E "*.wav" | sort -u -k1`;do
	uttpath=`echo ${x} | xargs -i dirname {}`
	spkid=${uttpath##*/}
	uttid=`basename ${x} | awk -F"." '{print $1}'`
	uttid=${spkid}_${uttid}
	echo "${uttid} ${x}" >> data/${out_set_name}/wav.scp
	echo "${uttid} ${spkid}" >> data/${out_set_name}/utt2spk
done
sort data/${out_set_name}/wav.scp -o data/${out_set_name}/wav.scp
sort data/${out_set_name}/utt2spk -o data/${out_set_name}/utt2spk
cat data/${out_set_name}/utt2spk | sort -k 2 | utils/utt2spk_to_spk2utt.pl > data/${out_set_name}/spk2utt || exit 1;

echo "finish data preparing for ${out_set_name}"
