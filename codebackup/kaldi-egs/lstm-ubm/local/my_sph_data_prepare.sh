#!/bin/bash

wav_dir=$1
data_set_name=$2

sph2pip=${KALDI_ROOT}/tools/sph2pipe_v2.5/sph2pipe

[ -d data/${data_set_name} ] || mkdir -p data/${data_set_name} || exit 1;

echo "clear data/${data_set_name}"
rm -rf data/${data_set_name}/{wav.scp,utt2spk,spk2utt}
echo "preparing scps in data/${data_set_name}"
for n in `find ${wav_dir}/${data_set_name}/*.wav | sort -u | xargs -i basename {} .wav`;do
	spkid=`echo ${n} | awk -F"_" '{print "" $1}'`
	uttid=${spkid}_${n}
	if [ -z ${sph2pip} ];then
		echo "${uttid} ${wav_dir}/${data_set_name}/${n}.wav" >> data/${data_set_name}/wav.scp
	else
		echo "${uttid} ${sph2pip} -f wav -p -c 1 ${wav_dir}/${data_set_name}/${n}.wav |" >> data/${data_set_name}/wav.scp
	fi
	echo "${uttid} ${spkid}" >> data/${data_set_name}/utt2spk
done
cat data/${data_set_name}/utt2spk | sort -k 2 | utils/utt2spk_to_spk2utt.pl > data/${data_set_name}/spk2utt || exit 1;

echo "finish data preparing for ${data_set_name}"
