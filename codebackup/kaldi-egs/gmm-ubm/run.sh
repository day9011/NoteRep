#!/bin/bash

. ./cmd.sh ## You'll want to change cmd.sh to something that will work on your system.
           ## This relates to the queue.
. ./path.sh

set -e

root=`pwd`  #exp home
num_threads=2      #parallel jobs num_job=2
num_components=2048
num_job=4
ivector_dim=600
num_iter=5
num_processes=1
num_gselect=20



data=~/Data/New-Netease-Data
mfccdir=`pwd`/mfcc
vaddir=`pwd`/mfcc

#data preparation

echo "Data dir: ${data}"
echo "number of gselect: ${num_gselect}"


local/my_amr_data_prepare_with_vad.sh ${data} train train || exit 1;



for x in train;do

	[ -d data/${x}_plp ] && rm -rf data/${x}_plp
	cp -rf data/${x} data/${x}_plp

	steps/make_mfcc.sh --mfcc-config conf/mfcc.conf --nj ${num_job} --cmd "${train_cmd}" \
		data/${x} exp/make_mfcc ${mfccdir}/vad_mfcc
	utils/fix_data_dir.sh data/${x}


	sid/compute_vad_decision.sh --vad-config conf/vad.conf --nj ${num_job} --cmd "${train_cmd}" \
		data/${x} exp/make_vad ${vaddir}
	utils/fix_data_dir.sh data/${x}

	steps/make_plp.sh --cmd "${train_cmd}" --nj ${num_job} --plp-config conf/plp.conf \
		data/${x}_plp exp/make_plp/${x} ${mfccdir}
	utils/fix_data_dir.sh data/${x}_plp

	steps/append_feats.sh --cmd "${train_cmd}" --nj ${num_job} \
		data/${x} data/${x}_plp data/${x}_data exp/make_append_feats ${mfccdir}
	utils/fix_data_dir.sh data/${x}_data

done


sid/train_diag_ubm.sh --cmd "${train_cmd}" \
	--nj ${num_job} --num-threads ${num_threads} --num-gselect ${num_gselect} \
	data/train_data ${num_components} exp/diag_ubm_${num_components}

sid/train_full_ubm.sh --cmd "${train_cmd}" --nj ${num_job} --remove-low-count-gaussians false --num-gselect ${num_gselect}  \
	data/train_data exp/diag_ubm_${num_components} exp/full_ubm_${num_components}


sid/train_ivector_extractor.sh --cmd "${train_cmd}" --ivector-dim ${ivector_dim} --num-iters ${num_iter} --num-processes ${num_processes} --nj ${num_job} --num-threads ${num_threads} --num-gselect ${num_gselect} \
	exp/full_ubm_${num_components}/final.ubm data/train_data exp/extractor

echo "finish model training"

for x in train;do
	sid/extract_ivectors.sh --cmd "${train_cmd}" --nj ${num_job} \
		exp/extractor data/${x}_data \
		exp/ivectors_${x}
done

ivector-compute-plda ark:data/train_data/spk2utt \
	'ark:ivector-normalize-length scp:exp/ivectors_train/ivector.scp ark:- |' \
	exp/ivectors_train/plda 2> exp/ivectors_train/plda.log

plda_ivec_dir=exp/ivectors_train

run.pl result/log/compute_mean.log \
	ivector-normalize-length scp:${plda_ivec_dir}/spk_ivector.scp ark:- \| ivector-mean ark:- ${plda_ivec_dir}/mean.vec || exit 1;


#echo "GMM-${num_components} EER"
#echo "fn plda eer: ${eer}" > result/fn_eer.txt
#echo fn plda eer: ${eer}

local/my_amr_data_prepare_with_vad.sh ${data} test test || exit 1;

for x in test;do

	[ -d data/${x}_plp ] && rm -rf data/${x}_plp
	cp -rf data/${x} data/${x}_plp

	steps/make_mfcc.sh --mfcc-config conf/mfcc.conf --nj ${num_job} --cmd "${train_cmd}" \
		data/${x} exp/make_mfcc ${mfccdir}/vad_mfcc
	utils/fix_data_dir.sh data/${x}


	sid/compute_vad_decision.sh --vad-config conf/vad.conf --nj ${num_job} --cmd "${train_cmd}" \
		data/${x} exp/make_vad ${vaddir}
	utils/fix_data_dir.sh data/${x}

	steps/make_plp.sh --cmd "${train_cmd}" --nj ${num_job} --plp-config conf/plp.conf \
		data/${x}_plp exp/make_plp/${x} ${mfccdir}
	utils/fix_data_dir.sh data/${x}_plp

	steps/append_feats.sh --cmd "${train_cmd}" --nj ${num_job} \
		data/${x} data/${x}_plp data/${x}_data exp/make_append_feats ${mfccdir}
	utils/fix_data_dir.sh data/${x}_data

done

for x in test;do
	sid/extract_ivectors.sh --cmd "${train_cmd}" --nj ${num_job} \
		exp/extractor data/${x}_data \
		exp/ivectors_${x}
done

trials_dir=`pwd`/data/trials
[ -d ${trials_dir} ] || mkdir -p ${trials_dir} || exit 1;

for x in test;do
	#compute fp eer
	trials=${trials_dir}/${x}_fp_trials

	[ -f ${trials} ] && rm ${trials}
	for i in `awk '{print $1}' "data/${x}_data/spk2utt"`
	do
		for j in `awk '{print $1}' "data/${x}_data/spk2utt"`
		do
			if [[ "${j}"x == "${i}"x ]];then
				echo "${i} ${j} target" >> ${trials}
			else
				echo "${i} ${j} nontarget" >> ${trials}
			fi
		done
	done

	echo "finish trials"

	[ -d result ] || mkdir result || exit 1;

	local/plda_scoring_filter_fp.sh data/train_data data/${x}_data data/${x}_data exp/ivectors_train exp/ivectors_${x} exp/ivectors_${x} ${trials} exp/score_${x}_fp_${num_components}

	echo "GMM-${num_components} EER"
	eer=`compute-eer <(python local/prepare_for_eer.py ${trials} exp/score_${x}_fp_${num_components}/plda_scores) 2> result/${x}_fp.log`
	echo "${x}_fp plda eer: ${eer}" > result/${x}_fp_eer.txt
	echo ${x}_fp plda eer: ${eer}


	#compute fn eer
	trials=${trials_dir}/${x}_fn_trials

	[ -f ${trials} ] && rm ${trials}
	for i in `awk '{print $1}' "data/${x}_data/utt2spk"`
	do
		for j in `awk '{print $1}' "data/${x}_data/utt2spk"`
		do
			i1=`echo ${i}| awk -F"_" '{print $1}'`
			j1=`echo ${j}| awk -F"_" '{print $1}'`
			if [[ "${j1}"x == "${i1}"x ]];then
				echo "${i} ${j} target" >> ${trials}
			else
				echo "${i} ${j} nontarget" >> ${trials}
			fi
		done
	done

	echo "finish trials"

	[ -d result ] || mkdir result || exit 1;

	local/plda_scoring_filter_fn.sh data/train_data data/${x}_data data/${x}_data exp/ivectors_train exp/ivectors_${x} exp/ivectors_${x} ${trials} exp/score_${x}_fn_${num_components}

	echo "GMM-${num_components} EER"
	eer=`compute-eer <(python local/prepare_for_eer.py ${trials} exp/score_${x}_fn_${num_components}/plda_scores) 2> result/${x}_fn.log`
	echo "${x}_fn plda eer: ${eer}" > result/${x}_fn_eer.txt
	echo ${x}_fn plda eer: ${eer}
done
