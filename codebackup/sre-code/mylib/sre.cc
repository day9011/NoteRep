#include "sre.h"

template<class DataType>
std::string SRE<DataType>::get_filename()
{
	return filename_;
}

template<class DataType>
void SRE<DataType>::set_filename(std::string filename)
{
	filename_ = filename;
}

template<class DataType>
bool SRE<DataType>::data_read()
{
	try
	{
		DataType data_holder_;
		data_holder_.Read(filename_, opts_.custom_opts.channel, opts_.mfcc_opts.frame_opts.samp_freq);
		Vector<BaseFloat> tempdata(data_holder_.getData());
		int32 num_ignore = opts_.mfcc_opts.frame_opts.WindowShift() * opts_.custom_opts.num_ignore_frames;
		voice_data_ = tempdata.Range(num_ignore, tempdata.Dim() - num_ignore);
		KALDI_LOG << "ignore num samples:" << num_ignore;
		return true;
	}
	catch (...)
	{
		return false;
	}
}

template<class DataType>
bool SRE<DataType>::webrtc_vad()
{
	Vector<BaseFloat> tempdata(voice_data_);
//	uint8_t* in_data = reinterpret_cast<uint8_t*>(tempdata.Data());
	BaseFloat* in_data = tempdata.Data();
	std::vector<BaseFloat> out_data;
	if (!process_vad(in_data, sampFreq, 3, 30, 1, 16, tempdata.Dim(), out_data))
	{
		KALDI_LOG << "processing vad failed";
		return false;
	}
	int32 dim = out_data.size();
	Vector<BaseFloat> vaded_voice;
	vaded_voice.Resize(dim);
	vaded_voice.CopyFromPtr(&(out_data[0]), dim);
	voice_data_ = vaded_voice;
	KALDI_LOG << "finish webrtc vad";
	return true;
}

//compute mfcc
template<class DataType>
bool SRE<DataType>::compute_mfcc()
{
	BaseFloat vtln_warp = vtlnWarp;
	long long int start, end;
	start = getSystemTime();
//	int32 num_ignore = 0;	
	KALDI_LOG << "vtln_warp is: " << vtln_warp;
	Mfcc mfcc(opts_.mfcc_opts);
	BaseFloat vtln_warp_local = vtln_warp;
	try
	{
	    mfcc.Compute(voice_data_, vtln_warp_local, &features_, NULL);	

//		CompressedMatrix compress_mat(features_);
//		compress_mat.CopyToMat(&features_);
//		print_matrix<BaseFloat>(features_);
	}
	catch (...)
	{
		return false;
	}
	KALDI_LOG << features_.NumRows() << " frames were completely computed!";
	end = getSystemTime();
	KALDI_LOG << "compute mfcc time: " << end - start << "ms";
	return true;
}

//plp feature compute
template<class DataType>
bool SRE<DataType>::compute_plp()
{
	Plp plp(opts_.plp_opts);
	Matrix<BaseFloat> plp_feature;
	try{
		plp.Compute(voice_data_, vtlnWarp, &plp_feature, NULL);
		//paste mfcc feature and plp feature
		if (plp_feature.NumRows() != features_.NumRows())
		{
			KALDI_WARN << "error rows between plp feature and mfcc feature" << " plp feature rows:" << plp_feature.NumRows() << " and mfcc feature rows:" << features_.NumRows();
			return false;
		}
		Matrix<BaseFloat> temp_feats(features_);
		int32 rows = temp_feats.NumRows(),
		  	  cols = temp_feats.NumCols() + plp_feature.NumCols();
		features_.Resize(rows, cols);
		features_.Range(0, rows, 0, temp_feats.NumCols()).CopyFromMat(temp_feats);
		features_.Range(0, rows, temp_feats.NumCols(), plp_feature.NumCols()).CopyFromMat(plp_feature);
	}
	catch (...)
	{
		return false;
	}
	return true;
}

//vad计算
template<class DataType>
bool SRE<DataType>::compute_vad()
{
	long long int start, end;
	start = getSystemTime();
	Vector<BaseFloat> vad(features_.NumRows());
	vad_result_ = vad;
	ComputeVadEnergy(opts_.vad_opts, features_, &vad_result_);
	double sum = vad_result_.Sum();
	if (sum == 0.0)
		return false;
	end = getSystemTime();
	KALDI_LOG << "compute vad time: " << end - start << "ms";
	return true;
}

//计算1，2阶导数，提取有效音频帧
template<class DataType>
bool SRE<DataType>::add_feats()
{
	long long int start, end;
	start = getSystemTime();
	Matrix<BaseFloat> _feats = features_;
	ComputeDeltas(opts_.delta_opts, _feats, &features_);
	_feats = features_;
	Matrix<BaseFloat> cmvn_feat(_feats.NumRows(), _feats.NumCols(), kUndefined);
	SlidingWindowCmn(opts_.slid_opts, _feats, &cmvn_feat);
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
	features_ = voiced_feat;
	KALDI_LOG << features_.NumRows() << " frames is valid";
	end = getSystemTime();
	KALDI_LOG << "add deltas to features time: " << end - start << "ms";
	return true;
}





template<class DataType>
bool SRE<DataType>::fgmm_to_gselect_posterior(SREUBM &ubm)
{
	long long int start, end;
	start = getSystemTime();
	int32 num_gselect = opts_.custom_opts.num_gselect;
	int32 num_frames = features_.NumRows();
	int32 num_gauss = ubm.gmm_.NumGauss();
	BaseFloat min_post = opts_.custom_opts.min_post;
	if (num_gselect > num_gauss) num_gselect = num_gauss;
	std::vector<std::vector<int32> > gselect(features_.NumRows());
	ubm.gmm_.GaussianSelection(features_, num_gselect, &gselect);
	end = getSystemTime();
	if (gselect.size() != num_frames)
	{
		std::cout << "gselect information has wrong size " << gselect.size()
												<< " vs " << num_frames << std::endl;
		return false;
	}
	KALDI_LOG << "gmm select time: " << end - start << "ms";
	// Output out_post("cpu-gselect.txt", false, false);
	// for (int i = 0; i < gselect.size(); i++)
	// {
	//     out_post.Stream() << "[";
	//     for (int j = 0; j < gselect[0].size(); j++)
	//         out_post.Stream() << " " << gselect[i][j];
	//     out_post.Stream() << " ]\n";
	// }
//	KALDI_LOG << "gselect size:" << gselect.size() << "\tgselect[0] size:" << gselect[0].size();
	start = getSystemTime();
	Posterior post(features_.NumRows());
	double this_tot_loglike = 0;
	double post_scale = opts_.custom_opts.posterior_scale;
	bool utt_ok = true;
	for (int32 t = 0; t < num_frames; t++)
	{
		SubVector<BaseFloat> frame(features_, t);
		const std::vector<int32> &this_gselect = gselect[t];
		KALDI_ASSERT(!gselect[t].empty());
		Vector<BaseFloat> loglikes;
		ubm.fgmm_.LogLikelihoodsPreselect(frame, this_gselect, &loglikes);
		this_tot_loglike += loglikes.ApplySoftMax();
		if (fabs(loglikes.Sum() - 1.0) > 0.01)
		{
			utt_ok = false;
		}
		else
		{
			if (min_post != 0.0)
			{
				int32 max_index = 0; // in case all pruned away...
				loglikes.Max(&max_index);
				for (int32 i = 0; i < loglikes.Dim(); i++)
					if (loglikes(i) < min_post)
						loglikes(i) = 0.0;
				BaseFloat sum = loglikes.Sum();
				if (sum == 0.0)
				{
					loglikes(max_index) = 1.0;
				}
				else
				{
					loglikes.Scale(1.0 / sum);
				}
			}
			for (int32 i = 0; i < loglikes.Dim(); i++)
			{
				if (loglikes(i) != 0.0)
				{
					post[t].push_back(std::make_pair(this_gselect[i], loglikes(i)));
				}
			}
			KALDI_ASSERT(!post[t].empty());
		}
	}
	ScalePosterior(post_scale, &post);
	post_ = post;
	end = getSystemTime();
	KALDI_LOG << "compute posterior time: " << end - start << "ms";
	return utt_ok;
}

template<class DataType>
bool SRE<DataType>::extract_ivector(SREUBM &ubm)
{
	long long int start, end;
	Vector<double> e_ivector;
	double max_count_scale = 1.0;
	ScalePosterior(opts_.extractor_opts.acoustic_weight * max_count_scale, &post_);
	bool need_2nd_order_stats = false;
    IvectorExtractorUtteranceStats utt_stats(ubm.ie_.NumGauss(),
                                             ubm.ie_.FeatDim(),
                                             need_2nd_order_stats);
	start = getSystemTime();
	utt_stats.AccStats(features_, post_);
	end = getSystemTime();
	KALDI_LOG << "compute gamma_ X_ time: " << end - start << "ms";
	// out_mat_to_file(utt_stats.X_, "cpu-X.txt");
	// out_vec_to_file(utt_stats.gamma_, "cpu-gamma.txt");
	e_ivector.Resize(ubm.ie_.IvectorDim());
	e_ivector(0) = ubm.ie_.PriorOffset();
	start = getSystemTime();
	ubm.ie_.GetIvectorDistribution(utt_stats, &e_ivector, NULL);
	end = getSystemTime();
	KALDI_LOG << "extract ivector get distribution time:" << end - start << " ms" << std::endl;
	e_ivector(0) -= ubm.ie_.PriorOffset();
	ivector_result_ = Vector<BaseFloat>(e_ivector);
	return true;
}

template<class DataType>
Vector<BaseFloat> SRE<DataType>::get_ivector()
{
	return ivector_result_;
}

template<class DataType>
Matrix<BaseFloat> SRE<DataType>::get_feature()
{
	return features_;
}

template<class DataType>
SubVector<BaseFloat> SRE<DataType>::voice_intercept(int num_ignore){
	return voice_data_.Range(num_ignore, voice_data_.Dim() - num_ignore); 
}

template class SRE<PCMGetData>;
template class SRE<WAVGetData>;


