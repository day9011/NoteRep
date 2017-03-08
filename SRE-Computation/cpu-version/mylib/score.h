#ifndef MY_SCORE
#define MY_SCORE
#include "mylib/sre.h"

namespace kaldi
{

inline void trim(string &);

class ScoreDotProduct
{
	public:
		ScoreDotProduct() {}
		~ScoreDotProduct() {}

		float score_dot_product(std::string file1, std::string file2);
};

}

#endif
