#ifndef MY_CUDA_FGMM
#define MY_CUDA_FGMM
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
#include "matrix/matrix-lib.h"
#include "cudalib/my-cuda-data-struct.h"
#include "cudalib/my-cuda-function.h"
#include "hmm/posterior.h"
#include "cudalib/my-cuda-tool.h"
#include <vector>

namespace kaldi
{


class CudaFGMM
{
	public:
		CudaFGMM() {}
		~CudaFGMM() {}
		void Read(std::istream &is, bool binary);
		void LogLikelihoodsPreselect(const CuMatrixBase<BaseFloat> &feature,
									 const CuMatrixIntBase &gselect,
									 CuMatrixBase<BaseFloat> *loglikes) const;

		void ComputePosterior(CuMatrixBase<BaseFloat> *loglikes, BaseFloat min_post);

		int32 Dim() const { return means_invcovars_.NumCols(); }
		int32 NumGauss() const { return means_invcovars_.NumRows(); }
		CuMatrix<BaseFloat> GetMeans() const { return means_invcovars_; }
		CuNumSpMatrix<BaseFloat> GetInvCovars() const { return cu_inv_covars_; }

	private:
		void ResizeInvCovars(int32 nmix, int32 dim);
		CuVector<BaseFloat> gconsts_;
		CuVector<BaseFloat> weights_;
		CuNumSpMatrix<BaseFloat> cu_inv_covars_;
		std::vector<CuSpMatrix<BaseFloat> > inv_covars_;
		CuMatrix<BaseFloat> means_invcovars_;
};

}
#endif
