#include "mylib/sre.h"
#include "ivector/plda.h"
void trim(string &);

void trim(string &s){
	if (s.empty()) return;
	int index = 0;
	for(; index < s.size(); index++)
		if(char(s[index]) < 58 && char(s[index]) > 47) break;
	s.erase(0, index);
	for(index = s.size() - 1; index > -1; index--)
		if(char(s[index] < 58) && char(s[index] > 47)) break;
	s.erase(index + 1, s.size());
}

int main(int argc, char *argv[]) {
	using namespace kaldi;
	typedef kaldi::int32 int32;
	typedef kaldi::int64 int64;
	try {
		const char *usage =
			"Computes dot-products between two iVectors;\n"
			"Usage: wav_score <file1> <file2>\n";
		ParseOptions po(usage);
		PldaConfig plda_opts;
		PldaUnsupervisedAdaptorConfig plda_adptor_config;
	    plda_adptor_config.Register(&po);
		plda_opts.Register(&po);
		po.Read(argc, argv);
		if (po.NumArgs() != 2) {
			po.PrintUsage();
			return -1;
		}
		std::string file1 = po.GetArg(1), file2 = po.GetArg(2);
		std::fstream _file;
		_file.open(file1.c_str(), std::ios::in);
		if(!_file) {
			KALDI_WARN << "\ncan't find wav file" << std::endl;
			throw false;
		}
		_file.close();
		_file.open(file2.c_str(), std::ios::in);
		if(!_file) {
			KALDI_WARN << "\ncan't find wav file" << std::endl;
			throw false;
		}
		_file.close();
		Input k1(file1), k2(file2);
		std::string text1, text2;
		std::string line;
		while (std::getline(k1.Stream(), line)) {
			text1 += line;
		}
		while (std::getline(k2.Stream(), line)) {
			text2 += line;
		}
		if (text1.empty() || text2.empty()) {
			KALDI_WARN << "empty ivector file" << std::endl;
			return -1;
		}
		trim(text1);
		trim(text2);
		std::vector<BaseFloat> fields;
		SplitStringToFloats(text1, " \t\n\r", true, &fields);
		Vector<BaseFloat> ivector1;
		ivector1.Resize(fields.size());
		ivector1.CopyFromPtr(&fields[0], fields.size());
		SplitStringToFloats(text2, " \t\n\r", true, &fields);
		Vector<BaseFloat> ivector2;
		ivector2.Resize(fields.size());
		ivector2.CopyFromPtr(&fields[0], fields.size());
		BaseFloat dot_prod = VecVec(ivector1, ivector2);
		std::cout << dot_prod << std::endl;
	} catch(const std::exception &e) {
		std::cerr << e.what();
		std::cout << "error" << std::endl;
		return -1;
	}
}
