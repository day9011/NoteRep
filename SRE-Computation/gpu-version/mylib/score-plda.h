#ifndef MY_PLDA_SCORE
#define MY_PLDA_SCORE
#include <vector>
#include <algorithm>
#include "ivector/plda.h"
#include "mylib/sre.h"
#include "mylib/ubm.h"
#include "mylib/conf.h"

using namespace std;

namespace kaldi
{

class ScorePLDA
{
	public:
		ScorePLDA() {}
		~ScorePLDA() {}

		float score_plda(SREUBM* ubm, std::string enroll_ivector_path, std::string test_ivector_path, std::string plda_mean_vec_path);
};

}

#endif
