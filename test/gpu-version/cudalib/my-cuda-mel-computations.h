//cuda version for mel-computations
#ifndef MY_CUDA_MEL_COMPUTATIONS
#define MY_CUDA_MEL_COMPUTATIONS
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <complex>
#include <utility>
#include <vector>

#include "base/kaldi-common.h"
#include "util/common-utils.h"
#include "matrix/matrix-lib.h"
#include "feat/feature-functions.h"
#include "cudamatrix/cu-common.h"
#include "cudamatrix/cu-value.h"
#include "cudamatrix/cu-matrix.h"
#include "cudamatrix/cu-vector.h"
#include "cudalib/my-cuda-function-kernel.h"

namespace kaldi{
class MelTest{
public:
	void test();
private:
	int i;
};
#if HAVE_CUDA==1

template<typename Real>
class CuMelBanks{  // cuda version for MelBanks class
public:
	static inline Real InverseMelScale(Real mel_freq) {
    return 700.0f * (expf (mel_freq / 1127.0f) - 1.0f);
  }

    static inline Real MelScale(Real freq) {
    return 1127.0f * logf (1.0f + freq / 700.0f);
  }
	void Compute(const CuMatrix<Real> &power_spectrum, CuMatrix<Real> &mel_energies_out) const; // computer energy for each frame
	void Compute(); // test
	CuMelBanks();
	CuMelBanks(const MelBanksOptions &opts, const FrameExtractionOptions &frame_opts, Real vtln_warp_factor);

    int32 NumRows() const { return bins_.NumRows(); }
	int32 NumCols() const { return bins_.NumCols(); }

  // returns vector of central freq of each bin; needed by plp code.
  const CuVector<Real> &GetCenterFreqs() const { return center_freqs_; }


private: 
	const static int32_cuda cols = 20; // default value for width of mel banks
	int32_cuda num_bins;
	int32_cuda *offset_;
	int32_cuda *bins_len_;
	CuMatrix<Real> bins_; // the number of values in each row is different
	CuVector<Real> center_freqs_;


    bool debug_;
    bool htk_mode_;
    //KALDI_DISALLOW_COPY_AND_ASSIGN(CuMelBanks);

};
}
#endif
#endif
