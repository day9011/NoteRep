#include "mylib/score.h"

namespace kaldi
{

inline void trim(string &s){
	if (s.empty()) return;
	int index = 0;
	for(; index < s.size(); index++)
		if(char(s[index]) < 58 && char(s[index]) > 47) break;
	s.erase(0, index);
	for(index = s.size() - 1; index > -1; index--)
		if(char(s[index] < 58) && char(s[index] > 47)) break;
	s.erase(index + 1, s.size());
}

float ScoreDotProduct::score_dot_product(std::string file1, std::string file2)
{
	std::fstream _file;
	_file.open(file1.c_str(), std::ios::in);
	if(!_file) {
		KALDI_WARN << "\ncan't find file" << std::endl;
		return -9999.0;
	}
	_file.close();
	_file.open(file2.c_str(), std::ios::in);
	if(!_file) {
		KALDI_WARN << "\ncan't find file" << std::endl;
		return -9999.0;
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
		return -9999.0;
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
	return static_cast<float>(dot_prod);
}

}
