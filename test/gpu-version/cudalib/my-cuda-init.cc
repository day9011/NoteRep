#include "cudalib/my-cuda-init.h"

bool CudaInitUBM::CudaInitReadFile(std::string final_ubm, std::string final_ie, std::string gmm_ubm, int gpuid)
{
	my_time init_time;
	init_time.start();
	long start, end;
#if HAVE_CUDA == 1
	gpuid_ = gpuid;
	start = getSystemTime();
	int32 n_gpu = 0;
	cudaGetDeviceCount(&n_gpu);
	if (n_gpu == 0 || gpuid > (n_gpu - 1) || gpuid < 0)
	{
		std::cout << "no avaliable gpu or wrong gpu id";
		return false;
	}
    CuDevice::Instantiate().SelectGpuIdManual(static_cast<int32>(gpuid));
    end = getSystemTime();
    KALDI_LOG << "select gpu:" << end - start << "ms";
#else
    {
		std::cout << "no cuda";
		return false;
	}
#endif
    my_time t;
    t.start();
    ReadKaldiObject(final_ie, &ie_);
    t.end();
    KALDI_LOG << "read final.ie file time: " << t.used_time() << "ms";
    t.start();
    ReadKaldiObject(final_ubm, &fgmm_);
    t.end();
    KALDI_LOG << "read final.ubm file time: " << t.used_time() << "ms";
    t.start();
    ReadKaldiObject(final_ubm, &cufgmm_);
    t.end();
    KALDI_LOG << "read cuda final.ubm file time: " << t.used_time() << "ms";
    t.start();
    ReadKaldiObject(gmm_ubm, &gmm_);
    t.end();
    KALDI_LOG << "read gmm.ubm file time: " << t.used_time() << "ms";
	init_time.end();
	KALDI_LOG << "cuda compute ivector init time:" << init_time.used_time() << "ms";
	return true;
}


int CudaInitUBM::getGpuID()
{
	return gpuid_;
}
