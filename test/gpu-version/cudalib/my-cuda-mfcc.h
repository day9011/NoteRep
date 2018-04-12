#ifndef CUDA_FEATURE_MFCC_H_
#define CUDA_FEATURE_MFCC_H_

#include <map>
#include <string>

#include "cudamatrix/cu-matrix.h"
#include "feat/feature-functions.h"
#include "feat/feature-mfcc.h"
#include "cudalib/my-cuda-mel-computations.h"
#include "cudalib/my-cuda-function.h"

namespace kaldi {

/// Class for computing MFCC features; see \ref feat_mfcc for more information.
class CuMfcc {
 public:
  explicit CuMfcc(const MfccOptions &opts);
  ~CuMfcc();

  int32 Dim() const { return opts_.num_ceps; }

  void Compute(const CuVectorBase<BaseFloat> &wave,
               CuMatrix<BaseFloat> &output);

  // Extract window from raw frame data;
  void ExtractWindow(CuMatrixBase<BaseFloat> &wave);

  // Perform DCT on mel energy
  void DCT(CuMatrix<BaseFloat> &feature);
  typedef MfccOptions Options;
 private:
  void ComputeInternal(const CuVectorBase<BaseFloat> &wave,
                       const CuMelBanks<BaseFloat> &mel_banks,
                       CuMatrix<BaseFloat> &output);
  
  MfccOptions opts_;
  CuMatrix<BaseFloat> cu_frames_;
  CuMatrix<BaseFloat> cu_mel_energy_;
  CuVector<BaseFloat> cu_lifter_coeffs_;
  CuMatrix<BaseFloat> cu_dct_matrix_;  // matrix we left-multiply by to perform DCT.
  BaseFloat log_energy_floor_;
  CuVector<BaseFloat> log_energy_;
  CuMelBanks<BaseFloat> *cu_mel_banks_;  // BaseFloat is VTLN coefficient.
  FeatureWindowFunction feature_window_function_;
  KALDI_DISALLOW_COPY_AND_ASSIGN(CuMfcc);
};


/// @} End of "addtogroup feat"
}  // namespace kaldi


#endif  // KALDI_FEAT_FEATURE_MFCC_H_

