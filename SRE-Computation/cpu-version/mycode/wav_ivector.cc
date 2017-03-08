#include "mylib/sre.h"
#include <string>

using namespace kaldi;
using kaldi::int32;
using std::vector;

int main(int argc, char *argv[])
{
	typedef kaldi::int64 int64; typedef kaldi::int32 int32;
	const char *usage = "Acquire test result for one person's wav.\n"
		"Usage: wav_ivector [options] <file.wav> <ubm-file> <ie-file> gmm.ubm valid_frames\n"
		"Options: --config\n"
		"example: wav_ivector --config=./mfcc.conf file final.ubm final.ie gmm.ubm valid_frames\n";
	ParseOptions po(usage);
	MyOption opts;
	opts.Register(&po);
	po.Read(argc, argv);
	if (po.NumArgs() != 5) {
		po.PrintUsage();
		exit(1);
	}
	KALDI_LOG << "melopts num bins:" << opts.mfcc_opts.mel_opts.num_bins;
	//数据处理阶段，输入输出替换成pair键值对
	std::string filename = po.GetArg(1);
	std::string final_ubm = po.GetArg(2);
	std::string final_ie = po.GetArg(3);
	std::string gmm_ubm = po.GetArg(4);
	std::string num_valid_frames_ = po.GetArg(5);
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
	SRE<WAVGetData> sre_instance(filename, final_ubm, final_ie, gmm_ubm);
	sre_instance.set_my_option(opts);
	//音频流数据读取
	if(!sre_instance.data_read())
	{
		KALDI_ERR << "read pcm file error!" << std::endl;
		return -1;
	};
	//mfcc特征值计算
	if(!sre_instance.compute_mfcc())
	{
		KALDI_ERR << "compute mfcc error!";
		return -1;
	}
//	print_matrix<BaseFloat>(sre_instance.get_feature());
	//vad计算
	if(!sre_instance.compute_vad())
	{
		KALDI_ERR << "compute vad error!";
		return -1;
	}
	//add-feats
	if(!sre_instance.add_feats())
	{
		KALDI_ERR << "add deltas to feats error!";
		return -1;
	}
	if (sre_instance.get_feature().NumRows() < num_valid_frames)
	{
		std::cout << "1";
		return -1;
	}
	KALDI_LOG << "input valid frames param:" << num_valid_frames;
	KALDI_LOG << "valid frames:" << sre_instance.get_feature().NumRows();
	//gmm select and compute posterior
	if(!sre_instance.fgmm_to_gselect_posterior())
	{
		KALDI_ERR << "compute posterior error!";
		return -1;
	}
	//ivector extract
	if(!sre_instance.extract_ivector())
	{
		KALDI_ERR << "extract ivector error!";
		return -1;
	}
	print_vector<BaseFloat>(sre_instance.get_ivector());
	KALDI_LOG << "\nfinished!";
	return 0;
}
