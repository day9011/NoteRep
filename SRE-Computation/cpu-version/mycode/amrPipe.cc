#include "mylib/cpu-compute.h"
#include "mylib/cpu-init.h"
#include <string>

struct wav_data_param
{
	int data_length;
	int sample_rate;
	int bits_per_sample;
	int channels;
}

static void write_string()

using namespace kaldi;
using kaldi::int32;
using std::vector;

int main(int argc, char *argv[])
{
	typedef kaldi::int64 int64; typedef kaldi::int32 int32;
	const char *usage = "Acquire test result for one person's pcm.\n"
		"Usage: wav_ivector [options] <file.pcm> <ubm-file> <ie-file> gmm.ubm ivector_path valid_frames\n"
		"Options: --config\n"
		"example: wav_ivector --config=./mfcc.conf file final.ubm final.ie gmm.ubm ivector_path valid_frames\n";
	ParseOptions po(usage);
	MyOption opts;
	opts.Register(&po);
	po.Read(argc, argv);
	if (po.NumArgs() != 6) {
		po.PrintUsage();
		exit(1);
	}
	KALDI_LOG << "melopts num bins:" << opts.mfcc_opts.mel_opts.num_bins;
	//数据处理阶段，输入输出替换成pair键值对
	std::string filename = po.GetArg(1);
	std::string final_ubm = po.GetArg(2);
	std::string final_ie = po.GetArg(3);
	std::string gmm_ubm = po.GetArg(4);
	std::string ivectorpath= po.GetArg(5);
	std::string num_valid_frames_ = po.GetArg(6);
	int32 num_valid_frames = static_cast<int32>(std::stoi(num_valid_frames_));
	std::fstream _file;
	_file.open(filename.c_str(), std::ios::in);
	if(!_file) {
		KALDI_ERR << "\ncan't find pcm file" << std::endl;
		return -1;
	}
	_file.close();
	_file.open(final_ubm.c_str(), std::ios::in);
	if(!_file) {
		KALDI_ERR << "\ncan't find final.ubm file" << std::endl;
		return -1;
	}
	_file.close();
	_file.open(final_ie.c_str(), std::ios::in);
	if(!_file) {
		KALDI_ERR << "\ncan't find final.ie file" << std::endl;
		return -1;
	}
	_file.close();
	_file.open(gmm_ubm.c_str(), std::ios::in);
	if(!_file) {
		KALDI_ERR << "\ncan't find gmm.ubm file" << std::endl;
		return -1;
	}
	_file.close();
	InitSRE ubm;
	ubm.ReadUBMFile(final_ubm, final_ie, gmm_ubm);
	CpuCompute comp;
	comp.Compute(&ubm, filename, ivectorpath, num_valid_frames);
	KALDI_LOG << "\nfinished!";
	return 0;
}
