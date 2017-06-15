#include "mylib/score-plda.h"

namespace kaldi
{

float ScorePLDA::score_plda(SREUBM* ubm, std::string enroll_ivector_path, std::string test_ivector_path, std::string plda_mean_vec_path)
{
	try{
		std::fstream _file;
		_file.open(enroll_ivector_path.c_str(), std::ios::in);
		if(!_file) {
			KALDI_WARN << "\ncan't find enroll_ivector file" << std::endl;
			return -9999.0;
		}
		_file.close();
		_file.open(test_ivector_path.c_str(), std::ios::in);
		if(!_file) {
			KALDI_WARN << "\ncan't find test_ivector file" << std::endl;
			return -9999.0;
		}
		_file.close();
		_file.open(plda_mean_vec_path.c_str(), std::ios::in);
		if(!_file) {
			KALDI_WARN << "\ncan't find plda_mean_vec file" << std::endl;
			return -9999.0;
		}
		Vector<BaseFloat> enroll_ivector, test_ivector, plda_mean_ivec;
		ReadKaldiObject(enroll_ivector_path, &enroll_ivector);
		ReadKaldiObject(test_ivector_path, &test_ivector);
		ReadKaldiObject(plda_mean_vec_path, &plda_mean_ivec);
		PldaConfig plda_opts;
		plda_opts.normalize_length = normalizeLength;
		plda_opts.simple_length_norm = simpleLengthNorm;
		my_ivector_normalize_length(enroll_ivector);
		enroll_ivector.AddVec(-1.0, plda_mean_ivec);
		my_ivector_normalize_length(enroll_ivector);
		my_ivector_normalize_length(test_ivector);
		test_ivector.AddVec(-1.0, plda_mean_ivec);
		my_ivector_normalize_length(test_ivector);
		int32 dim = ubm->plda_.Dim();
		Vector<BaseFloat> *transformed_enroll = new Vector<BaseFloat>(dim);
		Vector<BaseFloat> *transformed_test = new Vector<BaseFloat>(dim);
		ubm->plda_.TransformIvector(plda_opts, enroll_ivector, 1, transformed_enroll);
		ubm->plda_.TransformIvector(plda_opts, test_ivector, 1, transformed_test);
		Vector<double> enroll_ivec_dbl(*transformed_enroll);
		Vector<double> test_ivec_dbl(*transformed_test);
		BaseFloat score = ubm->plda_.LogLikelihoodRatio(enroll_ivec_dbl, 1, test_ivec_dbl);
		return score;
	}
	catch (...)
	{
		return -9999.0;
	}
}

}
