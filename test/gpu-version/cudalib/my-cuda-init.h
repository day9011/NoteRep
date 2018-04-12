#ifndef CUDA_INIT_UBM
#define CUDA_INIT_UBM

#include "cudalib/my-cuda-gmm.h"
#include "cudalib/my-cuda-fgmm.h"
#include "cudalib/my-cuda-ie.h"
#include "gmm/full-gmm.h"
#include "mylib/gettime.h"

namespace kaldi
{

class CudaInitUBM
{
	public:
		CudaInitUBM() : gpuid_(-1) {}
		bool CudaInitReadFile(std::string final_ubm, std::string final_ie, std::string gmm_ubm, int gpuid);
		~CudaInitUBM() {}
		int getGpuID();

		FullGmm fgmm_;
		CudaFGMM cufgmm_;
		CudaGMM gmm_;
		CudaIE ie_;
		int gpuid_;
};


}

#endif
