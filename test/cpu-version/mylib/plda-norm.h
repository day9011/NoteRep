#ifndef MY_PLDA_NORM_H
#define MY_PLDA_NORM_H

#include <vector>
#include <algorithm>
#include "ivector/plda.h"
#include "mylib/sre.h"

namespace kaldi
{

class PldaNorm: public Plda
{
	public:
		PldaNorm() {}
		double ScoreNorm(const Vector<double> &transform_train, int32 num_train_examles, const Vector<double> &transform_test, const Vector<double> &mean_imposter);

	private:
		Vector<double> get_diag_cov() { return psi_; }
		KALDI_DISALLOW_COPY_AND_ASSIGN(PldaNorm);
};



} //namespace kaldi

#endif
