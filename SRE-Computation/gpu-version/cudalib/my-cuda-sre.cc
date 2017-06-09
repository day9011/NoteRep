#include <iostream>
#include <fstream>
#include "cudalib/my-cuda-sre.h"

using namespace std;

namespace kaldi
{


template <class DataType>
CuMatrix<BaseFloat> CudaSRE<DataType>::get_cuda_feature()
{
	return cu_feature_;
}

template <class DataType>
Vector<double> CudaSRE<DataType>::get_ivector()
{
	KALDI_ASSERT(ivector_.Dim() > 0);
	if(ivector_result_.Dim() == 0 )
	{
		ivector_result_.Resize(ivector_.Dim(), kUndefined);
		ivector_.CopyToVec(&ivector_result_);
	}
	return ivector_result_;
}

template <class DataType>
bool CudaSRE<DataType>::setVoiceFileName(std::string filename)
{
	if (filename.empty())
		return false;
	else
		this->set_filename(filename);
	return true;
}

template<class DataType>
bool CudaSRE<DataType>::cuda_data_read(int32 channel, BaseFloat samp_freq){
	if (!this->data_read())
	{
		KALDI_WARN << "read pcm file error";
		return false;
	}
	int32 num_ignore = opts.mfcc_opts.frame_opts.WindowShift() * opts.custom_opts.num_ignore_frames;
		
	cu_voice_data_ = this->voice_intercept(num_ignore);
	return true;	
}

template <class DataType>
bool CudaSRE<DataType>::cuda_compute_vad(VadEnergyOptions vad_opts){
	Vector<BaseFloat> vad(cu_feature_.NumRows());
	vad_result_ = vad;
	ComputeVadEnergy(vad_opts, tmp_feature_, &vad_result_);
	double sum = vad_result_.Sum();
	if (sum == 0.0)
		return false;
	return true;
}

template<class DataType>
bool CudaSRE<DataType>::cuda_add_feats(SlidingWindowCmnOptions slid_opts, DeltaFeaturesOptions deltas_opts){
	Matrix<BaseFloat> _feats = tmp_feature_;
	ComputeDeltas(deltas_opts, _feats, &tmp_feature_);
	_feats = tmp_feature_;
	Matrix<BaseFloat> cmvn_feat(_feats.NumRows(), _feats.NumCols(), kUndefined);
	SlidingWindowCmn(slid_opts, _feats, &cmvn_feat);
	const Vector<BaseFloat> &voiced = vad_result_;
	int32 dim = 0;
	for (int32 i = 0; i < voiced.Dim(); i++)
		if (voiced(i) != 0.0)
			dim++;
	Matrix<BaseFloat> voiced_feat(dim, cmvn_feat.NumCols());
    int32 index = 0;
    for (int32 i = 0; i < cmvn_feat.NumRows(); i++)
	{
        if (voiced(i) != 0.0)
		{
	        KALDI_ASSERT(voiced(i) == 1.0); // should be zero or one.
            voiced_feat.Row(index).CopyFromVec(cmvn_feat.Row(i));
            index++;
        }
    }
	if(index != dim)
	{
		std::cout << "number of valid frames is error" << std::endl;
		return false;
	}
	tmp_feature_ = voiced_feat;
	KALDI_LOG << tmp_feature_.NumRows() << " frames is valid";
	return true;
}

template <class DataType>
bool CudaSRE<DataType>::cuda_compute_mfcc()
{
	clock_t start, stop;
	// test code
	/*
	std::ofstream zlog1("test1.log");
	this->data_read();
	this->compute_mfcc();
	this->compute_vad();
	this->add_feats();
	Matrix<BaseFloat> tmp(this->get_feature());
	for(int i = 0; i < tmp.NumRows(); i++){
		for(int j = 0; j < tmp.NumCols(); j++)
			zlog1 << tmp(i,j) << endl;
	}
	*/
		
	start = clock();
	if (!this->cuda_data_read(-1, opts.mfcc_opts.frame_opts.samp_freq))
	{
		KALDI_WARN << "cannot read data into cuda";
		return false;
	}
	stop = clock();
	KALDI_LOG << "CuDataRead time: " << (float)(stop - start) * 1000 / CLOCKS_PER_SEC << " ms";
	start = clock();	
	CuMfcc cu_mfcc(opts.mfcc_opts);
	cu_mfcc.Compute(cu_voice_data_, cu_feature_);
	stop = clock();
	KALDI_LOG << "CuMFCC time: " << (float)(stop - start) * 1000 / CLOCKS_PER_SEC << " ms";
	tmp_feature_.Resize(cu_feature_.NumRows(), cu_feature_.NumCols());
	tmp_feature_.CopyFromMat(cu_feature_);
	
	start = clock();	
	if (!this->cuda_compute_vad(opts.vad_opts))
	{
		KALDI_WARN << "compute vad";
		return false;
	}
	stop = clock();
	KALDI_LOG << "CuCompVad time: " << (float)(stop - start) * 1000 / CLOCKS_PER_SEC << " ms";
	
	start = clock();
	if (!this->cuda_add_feats(opts.slid_opts, opts.delta_opts))
	{
		KALDI_WARN << "add deltas to feature";
		return false;
	}
	stop = clock();

	KALDI_LOG << "CuDelta time: " << (float)(stop - start) * 1000 / CLOCKS_PER_SEC << " ms";	
	//test code
	/*
	std::ofstream zlog2("test2.log");
	Matrix<BaseFloat> tmp2(tmp_feature_);
	for(int i = 0; i < tmp2.NumRows(); i++){
		for(int j = 0; j < tmp2.NumCols(); j++)
			zlog2 << tmp2(i,j) << endl;
	}
	*/
	cu_feature_.Resize(tmp_feature_.NumRows(), tmp_feature_.NumCols());	

	cu_feature_.CopyFromMat(tmp_feature_);

#if FEATURE_TEST == 1
	Matrix<BaseFloat> out_mat(cu_feature_.NumRows(), cu_feature_.NumCols());
	cu_feature_.CopyToMat(&out_mat);
	std::string filename("gpu-feature.txt");
	out_mat_to_file(out_mat, filename);
	KALDI_LOG << "write feature result to file succesfully";
#endif
	return true;
}


template <class DataType>
bool CudaSRE<DataType>::gmm_select(CudaGMM &cu_gmm_)
{
	my_time t;
	t.start();
	int32 num_gselect = numGselect;
	cu_gmm_.GaussianSelection(cu_feature_, num_gselect, &cu_gselect_);
	t.end();
	KALDI_LOG << "GMM select time: " << t.used_time() << "ms";
	// out_mat_to_file(cu_gselect_, "gpu-gselect.txt");
#if GSELECT_TEST == 1
	std::string filename("gpu-gselect.txt");
	out_mat_to_file(cu_gselect_, filename);
	KALDI_LOG << "write gmm select result to file succesfully";
#endif
	return true;
}

template <class DataType>
bool CudaSRE<DataType>::cuda_compute_posterior(FullGmm &fgmm_)
{
	my_time t;
	t.start();
	Matrix<BaseFloat> frames(tmp_feature_);
	int32 num_frames = NumFrames();
	Posterior post(num_frames);
	for (int32 t = 0; t < num_frames; t++)
	{
		SubVector<BaseFloat> frame(frames, t);
		std::vector<int32> this_gselect(cu_gselect_.Data() + t * cu_gselect_.NumCols(), cu_gselect_.Data() + t * cu_gselect_.NumCols() + cu_gselect_.NumCols());
		KALDI_ASSERT(this_gselect.size() == cu_gselect_.NumCols());
		
		Vector<BaseFloat> loglikes;
		fgmm_.LogLikelihoodsPreselect(frame, this_gselect, &loglikes);
		loglikes.ApplySoftMax();
		if (fabs(loglikes.Sum() - 1.0) > 0.01)
			return false;
		else
		{
			if (minPosterior != 0.0)
			{
				int32 max_index = 0;
				loglikes.Max(&max_index);
				for (int32 i = 0; i < loglikes.Dim(); i++)
					if (loglikes(i) < minPosterior)
						loglikes(i) = 0;
				BaseFloat sum = loglikes.Sum();
				if (sum == 0.0)
					loglikes(max_index) = 1.0;
				else
					loglikes.Scale(1.0 / sum);
			}
			for (int32 i = 0; i < loglikes.Dim(); i++)
				if (loglikes(i) != 0.0)
					post[t].push_back(std::make_pair(this_gselect[i], loglikes(i)));
			KALDI_ASSERT(!post[t].empty());
		}
	}
	ScalePosterior(PosteriorScale, &post);
	cu_post_ = post;
	t.end();
	KALDI_LOG << "compute posterior time: " << t.used_time() << "ms";
#if POST_TEST == 1
	PosteriorHolder post_holder_;
	Output out_post("gpu-post.txt", false, false);
	post_holder_.Write(out_post.Stream(), false, cu_post_);
	KALDI_LOG << "write posterior result to file succesfully";
#endif
	return true;
}

template <class DataType>
bool CudaSRE<DataType>::cuda_compute_posterior(CudaFGMM &cufgmm_)
{
	my_time t;
	t.start();
	loglikes_.Resize(NumFrames(), cufgmm_.NumGauss());
	loglikes_.SetZero();
	cufgmm_.LogLikelihoodsPreselect(cu_feature_, cu_gselect_, &loglikes_);
	CudaMatApplySoftMax(&loglikes_);
	CudaComputePosterior(&loglikes_, opts.custom_opts.min_post);
	t.end();
	KALDI_LOG << "compute posterior time: " << t.used_time() << "ms";
	return true;
}

template <class DataType>
bool CudaSRE<DataType>::cuda_compute_ivector(CudaIE &ie)
{
	my_time t;
	t.start();
	ScalePosterior(1.0, &cu_post_);
	cu_stats_.Resize(ie.NumGauss(), ie.FeatDim());
	cu_stats_.AccStats(cu_feature_, loglikes_);
	ivector_.Resize(ie.IvectorDim());
	ivector_(0) = ie.PriorOffset();
	ie.GetIvectorDistribution(cu_stats_, &ivector_);
	ivector_(0) = ivector_(0) - ie.PriorOffset();
	cuda_ivector_normalize_length(ivector_);
	cuda_ivector_mean(ivector_);
	cuda_ivector_normalize_length(ivector_);
	t.end();
	KALDI_LOG << "compute ivector time: " << t.used_time() << "ms";
	return true;
}

template class CudaSRE<PCMGetData>;
template class CudaSRE<WAVGetData>;


}

