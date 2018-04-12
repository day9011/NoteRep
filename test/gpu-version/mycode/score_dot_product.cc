#include "mylib/score.h"

using namespace kaldi;
typedef kaldi::int32 int32;


int main(int argc, char *argv[])
{
	const char *usage = "Usage: score <file1> <file2>\n";
	ParseOptions po(usage);
	po.Read(argc, argv);
	if (po.NumArgs() != 2)
	{
		po.PrintUsage();
		return -1;
	}
	std::string file1 = po.GetArg(1), file2 = po.GetArg(2);
	ScoreDotProduct p;
	KALDI_LOG << "score:" << p.score_dot_product(file1, file2);
	KALDI_LOG << "compute end";
	return 0;
}
