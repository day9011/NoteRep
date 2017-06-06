#include "mylib/cpu-compute.h"
#include "mylib/cpu-init.h"
#include <string>

using namespace kaldi;
using kaldi::int32;
using std::vector;

int main(int argc, char *argv[])
{
	typedef kaldi::int64 int64; typedef kaldi::int32 int32;
	const char *usage = "Test effect of vad algorithm of a wav.\n";
	ParseOptions po(usage);
	MyOption opts;
	opts.Register(&po);
	po.Read(argc, argv);
	if (po.NumArgs() != 1) {
		po.PrintUsage();
		exit(1);
	}
	KALDI_LOG << "melopts num bins:" << opts.mfcc_opts.mel_opts.num_bins;
	//数据处理阶段，输入输出替换成pair键值对
	std::string filename = po.GetArg(1);
	std::fstream _file;
	_file.open(filename.c_str(), std::ios::in);
	if(!_file) {
		KADLI_WARN << "\ncan't find pcm file" << std::endl;
		return -1;
	}
	_file.close();
	SRE<PCMGetData> sre_inc;
	sre_inc.set_filename(filename);
	sre_inc.data_read();
	// Vector<BaseFloat> voice_vec = sre_inc.get_voice_data();
	// print_vector(voice_vec);
	// printf("\n");
	// std::cout << "voice vector" << std::endl;
	sre_inc.compute_mfcc();
	sre_inc.compute_vad();
	Vector<BaseFloat> vad_res = sre_inc.get_vad_result();
	print_vector(vad_res);
	printf("\n");
	std::cout << "vad result" << std::endl;
	KALDI_LOG << "\nfinished!";
	return 0;
}
