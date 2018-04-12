#ifndef SRE_H
#define SRE_H

#include "ivector/ivector-extractor.h"
#include "hmm/posterior.h"
#include "gmm/full-gmm.h"
#include "gmm/mle-full-gmm.h"
#include "feat/feature-functions.h"
#include "base/kaldi-common.h"
#include "util/common-utils.h"
#include "feat/feature-mfcc.h"
#include "feat/feature-plp.h"
#include "matrix/kaldi-matrix.h"
#include "ivector/voice-activity-detection.h"
#include "gmm/diag-gmm.h"
#include "hmm/transition-model.h"
#include "gmm/am-diag-gmm.h"
#include "util/kaldi-io.h"
#include "mylib/pcm-data.h"
#include "mylib/wav-data.h"
#include "mylib/gettime.h"
#include "mylib/option.h"
#include "mylib/ubm.h"
#include "mylib/tool.h"
#include "mylib/fvadfunc.h"

using namespace kaldi;
using kaldi::int32;
using kaldi::int64;



//打印矩阵
template <typename T>
void print_matrix(const MatrixBase<T> &_matrix) {
	for (int i = 0; i < _matrix.NumRows(); i++) {
		std::cout << "[";
		for (int j = 0; j < _matrix.NumCols(); j++)
			std::cout << " " << _matrix(i, j);
		std::cout << " ]\n";
	}
}

//打印向量
template <typename T>
void print_vector(const VectorBase<T> &_vector) {
	std::cout << "[";
	for (int i = 0; i < _vector.Dim(); i++)
		std::cout << " " << *(_vector.Data() + i);
	std::cout << " ]";
}

template <typename T>
void print_std_vector(std::vector<T> vec) {
	std::cout << "[ ";
	typename std::vector<T>::iterator it;
	for (it = vec.begin(); it != vec.end(); it++)
		std::cout << *it << " ";
	std::cout << "]\n";
}

template <typename T>
bool my_ivector_normalize_length(Vector<T> &ivector)
{
	long long start, end;
	start = getSystemTime();
	BaseFloat norm = ivector.Norm(2.0);
	BaseFloat ratio = norm / sqrt(ivector.Dim());
	KALDI_LOG << "Ratio is " << ratio << std::endl;
	if (ratio == 0.0)
	{
		std::cout << "Zero iVector" << std::endl;
		return false;
	}
	else ivector.Scale(1.0 / ratio);
	end = getSystemTime();
	KALDI_LOG << "ivector normalize length time: " << end - start << "ms";
	return true;
}

template <typename T>
bool my_ivector_mean(std::vector<Vector<T> > &ivectors, Vector<T> &ivector_mean)
{
	long long start, end;
	start = getSystemTime();
	Vector<T> spk_mean(ivectors[0].Dim());
	spk_mean.SetZero();
	for (auto iter = ivectors.begin(); iter != ivectors.end(); ++iter)
	{
		if ((*iter).Dim() < 1)
		{
			KALDI_WARN << "empty ivector";
			return false;
		}
		if (iter->Dim() != spk_mean.Dim())
		{
			KALDI_WARN << "error dim in ivector mean computing";	
			return false;
		}
		spk_mean.AddVec(1.0, (*iter));
	}
	spk_mean.Scale(1.0 / static_cast<T>(ivectors.size()));
	ivector_mean = spk_mean;
	end = getSystemTime();
	KALDI_LOG << "ivector mean time: " << end - start << "ms";
	return true;
}



template<class DataType>
class SRE {
	private:
		std::string filename_;
		Vector<BaseFloat> voice_data_;
		Matrix<BaseFloat> features_;
		Vector<BaseFloat> vad_result_;
		Posterior post_;
		Vector<BaseFloat> ivector_result_;
		MyOption opts_;
	public:
		friend class SRE<PCMGetData>;
		friend class SRE<WAVGetData>;
		bool add_feats();
		bool data_read();
		bool webrtc_vad();
		bool compute_mfcc();
		bool compute_vad();
		bool compute_plp();
		bool fgmm_to_gselect_posterior(SREUBM &ubm);
		bool extract_ivector(SREUBM &ubm);
		Vector<BaseFloat> get_ivector();
		Matrix<BaseFloat> get_feature();
		std::string get_filename();
		void set_filename(std::string filename);
		SubVector<BaseFloat> voice_intercept(int num_ignore);
		SRE() {}

		SRE(MyOption opts) { opts_ = opts; }

		void set_my_option(MyOption opts) { opts_ = opts; }
		Vector<BaseFloat> get_voice_data() { return voice_data_; }
		Vector<BaseFloat> get_vad_result() { return vad_result_; }

		~SRE() {}
};

#endif
