#include "mylib/plda-norm.h"

namespace kaldi
{


double PldaNorm::ScoreNorm(const Vector<double> &transform_train, int32 num_train_examles, const Vector<double> &transform_test, const Vector<double> &mean_imposter)
{
	if ((transform_train.Dim() != transform_test.Dim()) || (transform_train.Dim() != mean_imposter.Dim()))
	{
		KALDI_WARN << "error dim";
		return -10000;
	}
	Vector<double> temp_train(transform_train);
	Vector<double> temp_test(transform_test);
	temp_train.AddVec(-1.0, mean_imposter);
	temp_test.AddVec(-1.0, mean_imposter);
	my_ivector_normalize_length(temp_train);
	my_ivector_normalize_length(temp_test);
	BaseFloat score = this->LogLikelihoodRatio(temp_train, num_train_examles, temp_test);
	double score_res = static_cast<double>(score);
	score_res = 1000.0 * score_res / (sqrt(VecVec(temp_train, temp_train)) * VecVec(temp_test, temp_test));
	return score_res;
}

} // namespace kaldi
