#include "cudalib/my-cuda-init.h"
#include "cudalib/my-cuda-compute.h"

using namespace kaldi;
using kaldi::int32;

int main(int argc, char *argv[]){
	const char *usage = "./my_cuda wav_file final.ubm final.ie gmm.ubm valid_frames gpuid ivectorpath";
	ParseOptions po(usage);
	po.Read(argc, argv);
	if (po.NumArgs() != 7) {
		po.PrintUsage();
		exit(-1);
	}
	std::string voice_file = po.GetArg(1);
	std::string final_ubm = po.GetArg(2);
	std::string final_ie = po.GetArg(3);
	std::string gmm_ubm = po.GetArg(4);
	std::string valid_frames_ = po.GetArg(5);
	std::string gpuid_ = po.GetArg(6);
	std::string ivector_path = po.GetArg(7);
	int32 valid_frames = static_cast<int32>(std::stoi(valid_frames_));
	int32 gpuid = static_cast<int32>(std::stoi(gpuid_));
	my_time t;
	t.start();
	CudaInitUBM ubm;
	ubm.CudaInitReadFile(final_ubm, final_ie, gmm_ubm, gpuid);
	t.end();
	KALDI_LOG << "initialize gpu memory:" << t.used_time() << "ms";
	t.start();
	SRECompute comp;
	comp.cuda_sre_compute(&ubm, voice_file, ivector_path, valid_frames);
	t.end();
	KALDI_LOG << "total computation time: " << t.used_time() << "ms";
	SetVerboseLevel(4);
#if HAVE_CUDA == 1
	CuDevice::Instantiate().PrintProfile();
#endif
	return 0;
}
