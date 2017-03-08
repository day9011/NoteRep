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
#include "cudalib/my-cuda-mel-computations.h"
#include "cudalib/my-cuda-function.h"

using namespace std;
/*
void MelTest::test(){
	KALDI_LOG << "Just a test";
}
*/
#if HAVE_CUDA == 1

namespace kaldi{
// Notice: the cols of bins should be modified if using other mel, by default, cols = 20 should be enough
template<typename Real>
CuMelBanks<Real>::CuMelBanks(){
	cout << "CuMelBanks" << endl;
}

template<typename Real>
CuMelBanks<Real>::CuMelBanks(const MelBanksOptions &opts, const FrameExtractionOptions &frame_opts, Real vtln_warp_factor): htk_mode_(opts.htk_mode) {
  num_bins = opts.num_bins;
  if (num_bins < 3) 
  	KALDI_ERR << "Must have at least 3 mel bins";
  Real sample_freq = frame_opts.samp_freq;
  int32 window_length = static_cast<int32>(frame_opts.samp_freq*0.001*frame_opts.frame_length_ms);
  int32 window_length_padded =
      (frame_opts.round_to_power_of_two ?
       RoundUpToNearestPowerOfTwo(window_length) :
       window_length);
  KALDI_ASSERT(window_length_padded % 2 == 0);
  int32 num_fft_bins = window_length_padded/2;
  Real nyquist = 0.5 * sample_freq;

  Real low_freq = opts.low_freq, high_freq;
  if (opts.high_freq > 0.0)
    high_freq = opts.high_freq;
  else
    high_freq = nyquist + opts.high_freq;

  if (low_freq < 0.0 || low_freq >= nyquist
      || high_freq <= 0.0 || high_freq > nyquist
      || high_freq <= low_freq)
    KALDI_ERR << "Bad values in options: low-freq " << low_freq
              << " and high-freq " << high_freq << " vs. nyquist "
              << nyquist;
  
  Real fft_bin_width = sample_freq / window_length_padded;
  // fft-bin width [think of it as Nyquist-freq / half-window-length]

  Real mel_low_freq = MelScale(low_freq);
  Real mel_high_freq = MelScale(high_freq);

  debug_ = opts.debug_mel;

  // divide by num_bins+1 in next line because of end-effects where the bins
  // spread out to the sides.
  Real mel_freq_delta = (mel_high_freq - mel_low_freq) / (num_bins+1);

  Real vtln_low = opts.vtln_low,
      vtln_high = opts.vtln_high;
  if (vtln_high < 0.0) vtln_high += nyquist;
  
  if (vtln_warp_factor != 1.0 &&
      (vtln_low < 0.0 || vtln_low <= low_freq
       || vtln_low >= high_freq
       || vtln_high <= 0.0 || vtln_high >= high_freq
       || vtln_high <= vtln_low))
    KALDI_ERR << "Bad values in options: vtln-low " << vtln_low
              << " and vtln-high " << vtln_high << ", versus "
              << "low-freq " << low_freq << " and high-freq "
              << high_freq;

  Real *t_bins_ = (Real *)malloc(sizeof(Real) * num_bins * cols)
;
  
  
  Real *t_center_freqs_ = (Real *)malloc(sizeof(Real) * num_bins);
  int *t_offset_ = (int *)malloc(sizeof(int) * num_bins);
  int *t_bins_len_ = (int *)malloc(sizeof(int) * num_bins);

  bins_.Resize(num_bins, cols);
  center_freqs_.Resize(num_bins);
  cudaMalloc((void **)&offset_, sizeof(int32_cuda) * num_bins);
  cudaMalloc((void **)&bins_len_, sizeof(int32_cuda) * num_bins);
  
  for (int32 bin = 0; bin < num_bins; bin++) {
  	Real left_mel = mel_low_freq + bin * mel_freq_delta,
        center_mel = mel_low_freq + (bin + 1) * mel_freq_delta,
        right_mel = mel_low_freq + (bin + 2) * mel_freq_delta;
    // Here we don't consider vtln
	/*
    if (vtln_warp_factor != 1.0) {
      left_mel = VtlnWarpMelFreq(vtln_low, vtln_high, low_freq, high_freq,
                                 vtln_warp_factor, left_mel);
      center_mel = VtlnWarpMelFreq(vtln_low, vtln_high, low_freq, high_freq,
                                 vtln_warp_factor, center_mel);
     right_mel = VtlnWarpMelFreq(vtln_low, vtln_high, low_freq, high_freq,
                                  vtln_warp_factor, right_mel);
    }
	*/
    *(t_center_freqs_ + bin) = InverseMelScale(center_mel);
    // this_bin will be a vector of coefficients that is only
    // nonzero where this mel bin is active.
    Vector<BaseFloat> this_bin(num_fft_bins);
    int32 first_index = -1, last_index = -1;
    for (int32 i = 0; i < num_fft_bins; i++) {
      BaseFloat freq = (fft_bin_width * i);  // center freq of this fft bin.
      Real mel = MelScale(freq);
      if (mel > left_mel && mel < right_mel) {
        Real weight;
        if (mel <= center_mel)
          weight = (mel - left_mel) / (center_mel - left_mel);
        else
         weight = (right_mel-mel) / (right_mel-center_mel);
        this_bin(i) = weight;
        if (first_index == -1)
          first_index = i;
        last_index = i;
      }
    }
    KALDI_ASSERT(first_index != -1 && last_index >= first_index && "You may have set --num-mel-bins too large.");
   
//    KALDI_LOG << "Current bin: " << bin << " offset: " << first_index << " data: " << this_bin.Range(first_index, last_index + 1 - first_index); 
	*(t_offset_ + bin) = first_index;
	*(t_bins_len_ + bin) = last_index + 1 - first_index;
	memcpy(t_bins_ + bin * cols, this_bin.Data() + first_index, sizeof(Real) * (*(t_bins_len_ + bin)));
	
	/*
    bins_[bin].first = first_index;
    int32 size = last_index + 1 - first_index;
    bins_[bin].second.Resize(size);
    bins_[bin].second.CopyFromVec(this_bin.Range(first_index, size));
	*/
    // Replicate a bug in HTK, for testing purposes.
    if (opts.htk_mode && bin == 0 && mel_low_freq != 0.0)
      //bins_[bin].second(0) = 0.0;
	  *(t_bins_ + bin * cols) = 0; 
  }
  cudaMemcpy(bins_.Data(), t_bins_, sizeof(Real) * num_bins * cols, cudaMemcpyHostToDevice);
  
   cudaMemcpy(center_freqs_.Data(), t_center_freqs_, sizeof(Real) * num_bins, cudaMemcpyHostToDevice);
	
	cudaMemcpy(offset_, t_offset_, sizeof(int32_cuda) * num_bins, cudaMemcpyHostToDevice);
	
	cudaMemcpy(bins_len_, t_bins_len_, sizeof(int32_cuda) * num_bins, cudaMemcpyHostToDevice);
	
	free(t_center_freqs_);
	free(t_offset_);
	free(t_bins_len_);
	free(t_bins_);
};

template <typename Real> 
void CuMelBanks<Real>::Compute(const CuMatrix<Real> &power_spectrum, CuMatrix<Real> &mel_energies_out) const{
//	KALDI_LOG << "Num bins: " << bins_.NumRows() << " frame_num: " << power_spectrum.NumRows() << " frame dim: " << power_spectrum.Stride();
	if (mel_energies_out.NumCols() != bins_.NumRows())
		mel_energies_out.Resize(power_spectrum.NumRows(), bins_.NumRows());
	CudaMelPower(power_spectrum, offset_, bins_len_, bins_, mel_energies_out, htk_mode_);
}

/*
template<typename Real>
void CuMelBanks<Real>::Compute(){
	KALDI_LOG <<" test";
}
*/


template class CuMelBanks<float>;
template class CuMelBanks<double>;
}

#endif
