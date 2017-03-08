#ifndef MY_CUDA_COMPUTE
#define MY_CUDA_COMPUTE

#include "cudalib/my-cuda-sre.h"
#include "cudalib/my-cuda-init.h"
#include "cudalib/my-cuda-tool.h"
#include "mylib/gettime.h"

namespace kaldi
{

class SRECompute
{
	public:
		SRECompute() {}
		~SRECompute() {}

		bool cuda_sre_compute(CudaInitUBM *ubm, std::string voice_file, std::string ivector_path, int valid_frames);

};

}

#endif
