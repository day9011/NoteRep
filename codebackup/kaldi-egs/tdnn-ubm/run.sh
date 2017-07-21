#!/bin/bash

. ./cmd.sh ## You'll want to change cmd.sh to something that will work on your system.
           ## This relates to the queue.
. ./path.sh

set -e

root=`pwd`  #exp home
num_threads=2      #parallel jobs num_job=2
num_components=2048
num_job=2
ivector_dim=600
num_iter=5
num_processes=1
num_gselect=20



data=~/Data/Netease-train-dnn
mfccdir=`pwd`/mfcc
vaddir=`pwd`/mfcc
#tri1 and tri2
numLeavesTri1=2500
numGaussTri1=15000

#tri3 and tri4
numLeavesMLLT=2500
numGaussMLLT=15000

#tri5
numLeavesSAT=4000
numGaussSAT=30000
stage=1
train_stage=-10
use_gpu=true
minibatch_size=512
parallel_opts="--gpu 2"
min_post=0.015
nnet2dir=exp/nnet2_online/nnet_ms_a
nnet=${nnet2dir}/final.mdl


#data preparation

echo "Data dir: ${data}"
echo "number of gselect: ${num_gselect}"

local/my_amr_data_prepare_with_text.sh ${data} train train || exit 1;
local/my_amr_data_prepare_with_text.sh ${data} test test || exit 1;


local/my_dict_prepare.sh ${data} data/train
utils/prepare_lang.sh data/local/dict "<unk>" data/local/lang_tmp data/local/lang
local/my_train_lm.sh data/train/text data/local/dict/lexicon.txt || exit 1;
mkdir -p data/graph
cp data/local/lm/srilm/netease.3.lm.gz data/graph/netease.3.lm.gz || exit 1;
utils/format_lm_sri.sh data/local/lang data/graph/netease.3.lm.gz data/lang || exit 1;

for x in train test;do
  cp -rf data/${x} data/${x}_asr
  steps/make_mfcc.sh --cmd "${train_cmd}" --nj ${num_job} --mfcc-config conf/mfcc_asr.conf data/${x}_asr exp/make_mfcc/${x}_asr ${mfccdir}
  utils/fix_data_dir.sh data/${x}_asr
  utils/validate_data_dir.sh data/${x}_asr
  steps/compute_cmvn_stats.sh data/${x}_asr exp/make_mfcc/${x}_asr ${mfccdir}
done

cp -rf data/lang data/lang_test_bg 2>/dev/null

#train mono
echo "mono training & decoding"
steps/train_mono.sh  --nj ${num_job} --cmd "${train_cmd}"  data/train_asr data/lang exp/mono

utils/mkgraph.sh data/lang_test_bg exp/mono exp/mono/graph


steps/decode.sh --nj ${num_job} --cmd "${train_cmd}" \
 exp/mono/graph data/test_asr exp/mono/decode_test

steps/align_si.sh --nj ${num_job} --cmd "${train_cmd}" \
 data/train_asr data/lang exp/mono exp/mono_ali



echo "tri1 : Deltas + Delta-Deltas Training"

# Train tri1, which is deltas + delta-deltas, on train data.
steps/train_deltas.sh --cmd "${train_cmd}" \
 ${numLeavesTri1} ${numGaussTri1} data/train_asr data/lang exp/mono_ali exp/tri1

utils/mkgraph.sh data/lang_test_bg exp/tri1 exp/tri1/graph


steps/decode.sh --nj ${num_job} --cmd "${train_cmd}" --config conf/decode.config \
 exp/tri1/graph data/test_asr exp/tri1/decode_test

steps/align_si.sh --nj ${num_job} --cmd "${train_cmd}" \
 data/train_asr data/lang exp/tri1 exp/tri1_ali


# Train tri2, which is deltas + delta-deltas, on train data.
echo "tri2 : Deltas + Delta-Deltas Training"

steps/train_deltas.sh --cmd "${train_cmd}" \
 ${numLeavesTri1} ${numGaussTri1} data/train_asr data/lang exp/tri1_ali exp/tri2

utils/mkgraph.sh data/lang_test_bg exp/tri2 exp/tri2/graph


steps/decode.sh --nj ${num_job} --cmd "${train_cmd}" --config conf/decode.config \
 exp/tri2/graph data/test_asr exp/tri2/decode_test

steps/align_si.sh --nj ${num_job} --cmd "${train_cmd}" \
 data/train_asr data/lang exp/tri2 exp/tri2_ali


# Train tri3, which is LDA+MLLT, on train data.
echo "tri3 : LDA + MLLT Training & Decoding"

steps/train_lda_mllt.sh --cmd "${train_cmd}" \
 --splice-opts "--left-context=3 --right-context=3" \
 ${numLeavesMLLT} ${numGaussMLLT} data/train_asr data/lang exp/tri2_ali exp/tri3

utils/mkgraph.sh data/lang_test_bg exp/tri3 exp/tri3/graph


steps/decode.sh --nj ${num_job} --cmd "${train_cmd}" --config conf/decode.config \
 exp/tri3/graph data/test_asr exp/tri3/decode_test

steps/align_fmllr.sh --nj ${num_job} --cmd "${train_cmd}" \
 data/train_asr data/lang exp/tri3 exp/tri3_ali || exit 1;



# Train tri4, which is SAT, on train data.
echo "tri4 : LDA + MLLT + SAT Training & Decoding"

steps/train_sat.sh --cmd "${train_cmd}" \
 ${numLeavesMLLT} ${numGaussMLLT} data/train_asr data/lang exp/tri3_ali exp/tri4

utils/mkgraph.sh data/lang_test_bg exp/tri4 exp/tri4/graph

steps/decode.sh --nj ${num_job} --cmd "${train_cmd}" --config conf/decode.config \
 exp/tri4/graph data/test_asr exp/tri4/decode_test

steps/align_fmllr.sh --nj ${num_job} --cmd "${train_cmd}" \
 data/train_asr data/lang exp/tri4 exp/tri4_ali || exit 1;

# Train tri5, which is SAT, on train data.
steps/train_sat.sh --cmd "$train_cmd" \
 ${numLeavesSAT} ${numGaussSAT} data/train_asr data/lang exp/tri4_ali exp/tri5

utils/mkgraph.sh data/lang_test_bg exp/tri5 exp/tri5/graph

steps/decode.sh --nj ${num_job} --cmd "${train_cmd}" --config conf/decode.config \
 exp/tri5/graph data/test_asr exp/tri5/decode_test

steps/align_fmllr.sh --nj ${num_job} --cmd "${train_cmd}" \
 data/train_asr data/lang exp/tri5 exp/tri5_ali || exit 1;

#Make data for trainning nnet
utils/copy_data_dir.sh data/train data/train_hires_asr
steps/make_mfcc.sh --nj ${num_job} --mfcc-config conf/mfcc_hires.conf \
		      --cmd "$train_cmd" data/train_hires_asr exp/make_train_hires/train $mfccdir || exit 1;


#Train nnet2
mkdir -p exp/nnet2_online
sid/nnet2/train_multisplice_accel2.sh --stage ${train_stage} \
	--feat-type raw \
	--splice-indexes "layer0/-2:-1:0:1:2 layer1/-1:2 layer3/-3:3 layer4/-7:2" \
	--num-epochs 12 \
	--num-hidden-layers 6 \
	--num-jobs-initial 1 --num-jobs-final 2 \
	--num-threads "${num_threads}" \
	--minibatch-size "${minibatch_size}" \
	--parallel-opts "${parallel_opts}" \
	--mix-up 10500 \
	--initial-effective-lrate 0.0015 --final-effective-lrate 0.00015 \
	--cmd "${train_cmd}" \
	--pnorm-input-dim 3500 \
	--pnorm-output-dim 350 \
	data/train_hires_asr data/lang exp/tri5_ali ${nnet2dir}

#local/nnet2/decode.sh --nj ${num_job} --cmd "${train_cmd}" --config conf/decode_dnn.config --feat-type raw \
#		exp/tri5/graph data/train_hires_asr ${nnet2dir}/decode_test ${nnet2dir}

local/my_amr_data_prepare_with_vad.sh ${data} test test_sup || exit 1;
local/my_amr_data_prepare_with_vad.sh ${data} dev dev_sup || exit 1;
local/my_amr_data_prepare_with_vad.sh ${data} train train_sup || exit 1;


#Train GMM-UBM and Test DNN model
for x in train_sup;do
  cp -r data/${x} data/${x}_dnn
  steps/make_mfcc.sh --cmd "${train_cmd}" --nj ${num_job} --mfcc-config conf/mfcc.conf data/${x} exp/make_mfcc/${x} ${mfccdir}
  utils/fix_data_dir.sh data/${x}
  sid/compute_vad_decision.sh --nj ${num_job} --cmd "${train_cmd}" data/${x} exp/make_vad ${vaddir}
  utils/fix_data_dir.sh data/${x}
done

for x in train_sup_dnn;do
  steps/make_mfcc.sh --cmd "${train_cmd}" --nj ${num_job} --mfcc-config conf/mfcc_hires.conf data/${x} exp/make_mfcc/${x} ${mfccdir}
  utils/fix_data_dir.sh data/${x}
done

for x in train_sup;do
	cp data/${x}/vad.scp data/${x}_dnn/vad.scp
	cp data/${x}/utt2spk data/${x}_dnn/utt2spk
	cp data/${x}/spk2utt data/${x}_dnn/spk2utt
	utils/fix_data_dir.sh data/${x}
	utils/fix_data_dir.sh data/${x}_dnn
done

sid/init_full_ubm_from_dnn.sh --cmd "${train_cmd}" --use-gpu ${use_gpu} --nj 2 \
	data/train_sup data/train_sup_dnn ${nnet} exp/full_ubm_sup


#Train i-vector extractor on supervised-GMM
sid/train_ivector_extractor.sh --cmd "${train_cmd}" \
	--ivector-dim ${ivector_dim} --num-iters ${num_iter} --nj ${num_job} --num-threads ${num_threads} --num-processes ${num_processes} \
	exp/full_ubm_sup/final.ubm data/train_sup exp/extractor_sup_gmm

#Train i-ivector extractor on DNN-UBM
sid/train_ivector_extractor_dnn.sh --cmd "${train_cmd}" --use-gpu ${use_gpu} --nj 2 \
	--nnet-job-opt "--mem 1G" --min-post ${min_post} --ivector-dim ${ivector_dim} --num-iters ${num_iter} --num-processes 1 \
	exp/full_ubm_sup/final.ubm ${nnet} data/train_sup data/train_sup_dnn exp/extractor_dnn

#extract i-vector on supervised-GMM UBM
sid/extract_ivectors.sh --cmd "${train_cmd}" --nj ${num_job} \
	exp/extractor_sup_gmm data/train_sup exp/ivectors_sup_train

sid/extract_ivectors.sh --cmd "${train_cmd}" --nj ${num_job} \
	exp/extractor_sup_gmm data/test_sup exp/ivectors_sup_test

sid/extract_ivectors.sh --cmd "${train_cmd}" --nj ${num_job} \
	exp/extractor_sup_gmm data/dev_sup exp/ivectors_sup_dev


#extract i-vector on DNN-UBM
sid/extract_ivectors_dnn.sh --cmd "${train_cmd}" --nj ${num_job} --use-gpu ${use_gpu} \
	exp/extractor_dnn ${nnet} data/train_sup data/train_sup_dnn exp/ivectors_dnn_train

sid/extract_ivectors_dnn.sh --cmd "${train_cmd}" --nj ${num_job} --use-gpu ${use_gpu} \
	exp/extractor_dnn ${nnet} data/test_sup data/test_sup_dnn exp/ivectors_sup_dnn_test

sid/extract_ivectors_dnn.sh --cmd "${train_cmd}" --nj ${num_job} --use-gpu ${use_gpu} \
	exp/extractor_dnn ${nnet} data/dev_sup data/dev_sup_dnn exp/ivectors_sup_dnn_dev


#train plda model
ivector-compute-plda ark:data/train_sup/spk2utt \
	'ark:ivector-normalize-length scp:exp/ivectors_sup_train/ivector.scp ark:- |' \
	exp/ivectors_sup_train/plda 2>exp/ivectors_sup_train/plda.log

ivector-compute-plda ark:data/train_sup_dnn/spk2utt \
	'ark:ivector-normalize-length scp:exp/ivectors_dnn_train/ivector.scp ark:- |' \
	exp/ivectors_dnn_train/plda 2>exp/ivectors_dnn_train/plda.log

run.pl result/log/compute_sup_mean.log \
	ivector-normalize-length scp:exp/ivectors_sup_train/spk_ivector.scp ark:- \| ivector-mean ark:- exp/ivectors_sup_train/mean.vec || exit 1;

run.pl result/log/compute_dnn_mean.log \
	ivector-normalize-length scp:exp/ivectors_dnn_train/spk_ivector.scp ark:- \| ivector-mean ark:- exp/ivectors_dnn_train/mean.vec || exit 1;



#scoring
trials_dir=`pwd`/data/trials

[ -d ${trials_dir} ] || mkdir -p ${trials_dir} || exit 1;
test_trials=${trials_dir}/dev_trials

[ -f ${test_trials} ] && rm ${test_trials}

for i in `awk '{print $1}' "data/train_sup/spk2utt"`
do
	for j in `awk '{print $1}' "data/dev_sup/utt2spk"`
	do
		j1=`echo ${j} | awk -F "_" '{print $1}'`
		if [[ "${j1}" == "${i}" ]];then
			echo "${i} ${j} target" >> ${test_trials}
		else
			echo "${i} ${j} nontarget" >> ${test_trials}
		fi
	done
done

echo "finish test trials"

#scoring
[ -d result ] || mkdir result || exit 1;

local/plda_scoring.sh data/train_sup data/train_sup data/dev_sup exp/ivectors_sup_train exp/ivectors_sup_train exp/ivectors_sup_dev ${test_trials} exp/score_sup_dev

echo "GMM-UBM EER"
dev_sup_eer=`compute-eer <(python local/prepare_for_eer.py ${test_trials} exp/score_sup_dev/plda_scores) 2> exp/score_sup_dev/compute_eer.log`
echo "plda sup dev eer: ${dev_sup_eer}" > result/dev_sup_eer.txt
echo plda sup dev eer: ${dev_sup_eer}

local/plda_scoring.sh data/train_sup_dnn data/train_sup_dnn data/dev_sup_dnn exp/ivectors_sup_dnn_train exp/ivectors_sup_dnn_train exp/ivectors_dnn_dev ${test_trials} exp/score_dnn_dev

echo "DNN-UBM EER"
dev_dnn_eer=`compute-eer <(python local/prepare_for_eer.py ${test_trials} exp/score_dnn_dev/plda_scores) 2> exp/score_dnn_dev/compute_eer.log`
echo "plda dnn dev eer: ${dev_dnn_eer}" > result/dev_dnn_eer.txt
echo plda dnn dev eer: ${dev_dnn_eer}

test_trials=${trials_dir}/test_fn_trials

[ -f ${test_trials} ] && rm ${trials}

for i in `awk '{print $1}' "data/test_sup/utt2spk"`
do
    for j in `awk '{print $1}' "data/test_sup/utt2spk"`
    do
        i1=`echo ${i} | awk -F"_" '{print $1}'`
        j1=`echo ${j} | awk -F"_" '{print $1}'`
        if [[ "${j1}"x == "${i1}"x ]];then
            echo "${i} ${j} target" >> ${test_trials}
        else
            echo "${i} ${j} nontarget" >> ${test_trials}
        fi
    done
done

local/plda_scoring_filter_fn.sh data/train_sup data/test_sup data/test_sup exp/ivectors_sup_train exp/ivectors_sup_test exp/ivectors_sup_test ${test_trials} exp/score_fn_sup_test

echo "GMM-UBM EER"
test_sup_eer=`compute-eer <(python local/prepare_for_eer.py ${test_trials} exp/score_fn_sup_test/plda_scores) 2> exp/score_fn_sup_test/compute_eer.log`
echo "plda dnn dev eer: ${test_sup_eer}" > result/test_sup_eer.txt
echo plda dnn dev eer: ${test_sup_eer}

local/plda_scoring_filter_fn.sh data/train_sup_dnn data/test_sup_dnn data/test_sup_dnn exp/ivectors_sup_dnn_train exp/ivectors_sup_dnn_test exp/ivectors_sup_dnn_test ${test_trials} exp/score_fn_sup_dnn_test

echo "GMM-UBM EER"
test_sup_dnn_eer=`compute-eer <(python local/prepare_for_eer.py ${test_trials} exp/score_fn_sup_dnn_test/plda_scores) 2> exp/score_fn_sup_dnn_test/compute_eer.log`
echo "plda dnn dev eer: ${test_sup_dnn_eer}" > result/test_sup_dnn_eer.txt
echo plda dnn dev eer: ${test_sup_dnn_eer}



