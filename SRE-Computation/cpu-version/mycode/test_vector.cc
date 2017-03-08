#include "mylib/sre.h"
#include "mylib/gettime.h"
#include <vector>

using namespace kaldi;
using kaldi::int32;

int main(int argc, char *argv[])
{
	const char *usage = "For a test to std::vector\n"
						"Usage: ./test_vector final_ie";
	ParseOptions po(usage);
	po.Read(argc, argv);
	if (po.NumArgs() != 1)
	{
		po.PrintUsage();
		return -1;
	}
	std::string final_ie = po.GetArg(1);
	MyIvectorExtractor ie;
	ReadKaldiObject(final_ie, &ie);
	my_time t;
	t.start();
	int32 rows = ie.Sigma_inv_M_[0].NumRows();
	int32 cols = ie.Sigma_inv_M_[0].NumCols();
	int32 sizes = ie.getSigmaInvM().size();
	std::vector<Matrix<double> > mat_vec = ie.getSigmaInvM();
	KALDI_LOG << "need memory " << sizes * rows * cols << " double";
	double *data = new double[rows * cols * sizes];
	KALDI_LOG << "Sigma_Inv_M_ rows:" << rows << "    cols:" << cols << "   size:" << sizes;
	for (int32 i = 0; i < sizes; i++)
	{
		for (int32 row = 0; row < rows; row++)
			for (int32 col = 0; col < cols; col++)
				*(data + i * rows * cols + row * rows + col) = mat_vec[i](row, col);
	}
	t.end();
	delete data;
	KALDI_LOG << "copy data to 3D data time: " << t.used_time() << "ms";
	return 0;
}
