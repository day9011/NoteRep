#include "feat/feature-functions.h"
#include "cudalib/my-cuda-function.h"
#include "cudalib/my-cuda-sre.cc"
#include "mylib/gettime.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>

using namespace kaldi;

int main() {
	long long int start, end;
	BaseFloat data[256];
	BaseFloat datad[256];
#if HAVE_CUDA == 1
	start = getSystemTime();
	CuDevice::Instantiate().SelectGpuIdManual(1);
//  CuDevice::Instantiate().SelectGpuId("yes");
	end = getSystemTime();
	KALDI_LOG << "select gpu:" << end - start << "ms";
#else
	{
		KALDI_LOG << "no cuda";
		return 0;
	}
#endif
	for (int i = 0; i < 256; i++)
		data[i] = static_cast<BaseFloat>(i);
	for (int i = 0; i < 256; i++)
		datad[i] = static_cast<BaseFloat>(i);
	start = getSystemTime();
	SplitRadixRealFft<BaseFloat> *srfft;
	std::vector<BaseFloat> temp_buf;
	srfft = new SplitRadixRealFft<BaseFloat>(8);
	srfft->Compute(data, true, &temp_buf);
	end = getSystemTime();
	printf("\nsrfft compute time: %lldms", end - start);
	printf("\n");
	for (int i = 0; i < 256; i++) {
		printf("[%d]=%f ", i, data[i]);
	}
	printf("\n");
	start = getSystemTime();
	CudaFFT<BaseFloat> fft(8);
	fft.compute(datad);
	end = getSystemTime();
	printf("\ncudafft compute time: %lldms", end - start);
	printf("\n");
	for (int i = 0; i < 256; i++) {
		printf("[%d]=%f ", i, datad[i]);
	}
	printf("\n");
	return 1;
}
