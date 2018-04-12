#ifndef MY_CUDA_GMM
#define MY_CUDA_GMM

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
#include "mylib/gettime.h"
#include "cudalib/my-cuda-function.h"
#include "cudalib/my-cuda-data-struct.h"
#include "cudalib/my-cuda-tool.h"

namespace kaldi
{


class CudaGMM
{
	public:
		CudaGMM() {}
		~CudaGMM() {}

		void Read(std::istream &is, bool binary);
		int32 NumGauss() const { return weights_.Dim(); }
		int32 Dim() const { return means_invvars_.NumCols(); }
		void GaussianSelection(const CuMatrixBase<BaseFloat> &data,
							   int32 num_gselect,
							   CuMatrixInt *output) const;
		void LoglikeLihoods(const CuMatrixBase<BaseFloat> &data,
							CuMatrix<BaseFloat> *loglikes) const;

	private:
		CuVector<BaseFloat> gconsts_;
		CuVector<BaseFloat> weights_;
		CuMatrix<BaseFloat> inv_vars_;
		CuMatrix<BaseFloat> means_invvars_;
};

}
#endif
