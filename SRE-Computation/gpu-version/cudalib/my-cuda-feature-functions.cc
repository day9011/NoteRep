#include "cudalib/my-cuda-feature-functions.h"
#include "cudalib/my-cuda-function-kernel.h"



void CuExtractWindow(const CuVectorBase<BaseFloat> &wave,
                   int32 num_frames,  
                   const FrameExtractionOptions &opts,
                   const FeatureWindowFunction &window_function,
                   CuMatrix<BaseFloat> *window,
                   BaseFloat *log_energy_pre_window){
  int32 frame_shift = opts.WindowShift();
  int32 frame_length = opts.WindowSize();
  KALDI_ASSERT(window_function.window.Dim() == frame_length);
  KALDI_ASSERT(frame_shift != 0 && frame_length != 0);

  CuVector<BaseFloat> wave_part(frame_length * num_frames);
  //KALDI_LOG << "frame_length: " << frame_length;
  KALDI_LOG << "frame_shift: " << frame_shift;
  if (opts.snip_edges) {
    int32 start = 0, end = frame_length * num_frames;
    KALDI_ASSERT(start >= 0 && end <= wave.Dim());
    wave_part.CopyFromVec(wave.Range(start, end));
  } else {
	// coda below should be parallelized
    // If opts.snip_edges = false, we allow the frames to go slightly over the
    // edges of the file; we'll extend the data by reflection.
    int32 mid = frame_shift * (f + 0.5),
        begin = mid - frame_length / 2,
        end = begin + frame_length,
        begin_limited = std::max<int32>(0, begin),
        end_limited = std::min(end, wave.Dim()),
        length_limited = end_limited - begin_limited;

    // Copy the main part.  Usually this will be the entire window.
    //wave_part.Range(begin_limited - begin, length_limited).
    //    CopyFromVec(wave.Range(begin_limited, length_limited));
    my_cuda_parallel_memcpy(window->Data(), wave.Data(), num_frames, opts.PaddedWindowSize(), frame_length, num_frames, frame_length); 
  KALDI_ASSERT(window != NULL);
  
  //SubVector<BaseFloat> window_part(*window, 0, frame_length);
  //window_part.CopyFromVec(wave_part);

  if (opts.dither != 0.0) Dither(&window_part, opts.dither);

  if (opts.remove_dc_offset)
    window_part.Add(-window_part.Sum() / frame_length);

  if (log_energy_pre_window != NULL) {
    BaseFloat energy = std::max(VecVec(window_part, window_part),
                                std::numeric_limits<BaseFloat>::min());
    *log_energy_pre_window = log(energy);
  }

  if (opts.preemph_coeff != 0.0)
    Preemphasize(&window_part, opts.preemph_coeff);

  window_part.MulElements(window_function.window);

  if (frame_length != frame_length_padded)
    SubVector<BaseFloat>(*window, frame_length,
                         frame_length_padded-frame_length).SetZero();
}



