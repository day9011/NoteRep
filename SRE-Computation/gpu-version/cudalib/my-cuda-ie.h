#ifndef MY_CUDA_IVECTOR_EXTRACTOR
#define MY_CUDA_IVECTOR_EXTRACTOR

#include <cstdlib>
#include "base/kaldi-common.h"
#include "util/common-utils.h"
#include "cudamatrix/cu-matrix.h"
#include "cudamatrix/cu-vector.h"
#include "cudamatrix/cu-device.h"
#include "cudamatrix/cu-sp-matrix.h"
#include "cudamatrix/cu-tp-matrix.h"
#include "cudamatrix/cu-packed-matrix.h"
#include "cudamatrix/cu-matrix-lib.h"
#include "hmm/posterior.h"
#include "mylib/sre.h"
#include "mylib/gettime.h"
#include "cudalib/my-cuda-data-struct.h"
#include "cudalib/my-cuda-tool.h"
#include "cudalib/my-cuda-function.h"
#include "cudalib/my-cuda-function-kernel.h"
#include <vector>

using namespace kaldi;
using kaldi::int32;


class CudaIE;
class CudaIEStats;

class CudaIE
{
	public:
		CudaIE(): prior_offset_(0.0) {}

		int32 FeatDim() const;
		int32 IvectorDim() const;
		int32 NumGauss() const;
		inline double PriorOffset() const { return prior_offset_; }

		void GetIvectorDistribution(const CudaIEStats &utt_stats,
									CuVectorBase<double> *mean) const;

		void GetIvectorDistMean(const CudaIEStats &utt_stats,
								CuVectorBase<double> *linear,
								CuSpMatrix<double> *quadratic) const;

		void GetIvectorDistPrior(const CudaIEStats &utt_stats,
								CuVectorBase<double> *linear,
								CuSpMatrix<double> *quadratic) const;

		std::vector<CuMatrix<double> > getMmat() { return M_; }
		
		~CudaIE() {}
		void Read(std::istream &is, bool binary);
	private:
		CuMatrix<double> w_;
		CuVector<double> w_vec_;
		std::vector<CuMatrix<double> > M_;
		CuNumMatrix<double> CuNum_M_;
		std::vector<CuSpMatrix<double> > Sigma_inv_;
		CuNumMatrix<double> CuNum_Sigma_inv_;
		double prior_offset_;
		CuVector<double> gconsts_;
		CuMatrix<double> U_;
		std::vector<CuMatrix<double> > Sigma_inv_M_;
		CuNumMatrix<double> CuNum_Sigma_inv_M_;
};

class CudaIEStats
{
	public:
		CudaIEStats() {}

		void Resize(int32 num_gauss, int32 feat_dim);
		void AccStats(const MatrixBase<BaseFloat> &feats, const Posterior &post);
		void Scale(double scale);
		void AccStats(const CuMatrixBase<BaseFloat> &feats, const CuMatrixBase<BaseFloat> &loglikes);

		~CudaIEStats() {}
	private:
		friend class CudaIE;
		CuVector<double> gamma_;
		CuMatrix<double> X_;
		CuLinearMatrix<double> L_X_;
};

#endif
