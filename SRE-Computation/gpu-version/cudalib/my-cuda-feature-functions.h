// Modify some of feat/feature-functions into cuda version

#ifndef MY_CUDA_FEATURE_FUNCTIONS
#define MY_CUDA_FEATURE_FUNCTIONS
#include <string>
#include <vector>
#include "feat/feature-functions.h"
#include "matrix/matrix-lib.h"
#include "cudamatrix/cu-matrix.h"
#include "cudamatrix/cu-vector.h"
#include "util/common-utils.h"
#include "base/kaldi-error.h"

//cuda version for extracting window
//input are all frames
//output are all windowed data on cuda
void CuExtractWindow(const CuVectorBase<BaseFloat> &wave,
                   int32 num_frames,  
                   const FrameExtractionOptions &opts,
                   const FeatureWindowFunction &window_function,
                   CuMatrix<BaseFloat> *window,
                   BaseFloat *log_energy_pre_window);

#endif
