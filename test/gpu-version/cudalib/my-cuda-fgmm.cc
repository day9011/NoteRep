#include "cudalib/my-cuda-fgmm.h"
#include <algorithm>
#include <functional>
#include <limits>
#include <string>
#include <queue>
#include <utility>
using std::pair;
#include <vector>
using std::vector;

void CudaFGMM::ResizeInvCovars(int32 nmix, int32 dim)
{
	KALDI_ASSERT(nmix > 0 && dim > 0);
	if (inv_covars_.size() != static_cast<size_t>(nmix))
		inv_covars_.resize(nmix);
	for (int32 i = 0; i < nmix; i++)
	{
		inv_covars_[i].Resize(dim);
		inv_covars_[i].SetUnit();
	}
}

void CudaFGMM::Read(std::istream &is, bool binary)
{
#if HAVE_CUDA == 1
	std::string token;
	ReadToken(is, binary, &token);
	if (token != "<FullGMMBegin>" && token != "<FullGMM>")
	{
		KALDI_WARN << "Expected <FullGMM>, got" << token;
		exit(-1);
	}
	ReadToken(is, binary, &token);
	if (token == "<GCONSTS>")
	{
		gconsts_.Read(is, binary);
		ExpectToken(is, binary, "<WEIGHTS>");
	}
	else
		if (token != "<WEIGHTS>")
		{
			KALDI_WARN << "FullGMM::Read, expected <WEIGHTS> or <GCONSTS>, got"
					  << token;
			exit(-1);
		}
	weights_.Read(is, binary);
	ExpectToken(is, binary, "<MEANS_INVCOVARS>");
	means_invcovars_.Read(is, binary);
	ExpectToken(is, binary, "<INV_COVARS>");
	int32 ncomp = weights_.Dim(), dim = means_invcovars_.NumCols();
	ResizeInvCovars(ncomp, dim);
	for (int32 i = 0; i < ncomp; i++)
		inv_covars_[i].Read(is, binary);
	ReadToken(is, binary, &token);
	if (token != "<FullGMMEnd>" && token != "</FullGMM>")
	{
		KALDI_WARN << "Expected </FullGMM>, got" << token;
		exit(-1);
	}
	cu_inv_covars_.Resize(ncomp, dim);
	cu_inv_covars_.CopyFromMats(inv_covars_);
#else
	KALDI_WARN << "No Cuda";
	exit(-1);
#endif
}

void CudaFGMM::LogLikelihoodsPreselect(const CuMatrixBase<BaseFloat> &feature,
							 const CuMatrixIntBase &gselect,
							 CuMatrixBase<BaseFloat> *loglikes) const
{
	int32 dim = Dim();
	KALDI_ASSERT(dim == feature.NumCols());
	my_time t;
	t.start();
	CuNumSpMatrix<BaseFloat> data_sqs(feature.NumRows(), dim);
	KALDI_LOG << "data_sqs nums:" << data_sqs.NumSizes() << "    rows:" << data_sqs.NumRows() << "    d.stride:" << data_sqs.Dim().stride;

	data_sqs.AddVec3(feature, BaseFloat(1.0));
	t.end();
	KALDI_LOG << "compute AddVec3 time: " << t.used_time() << "ms";
	data_sqs.ScaleDiagNum(BaseFloat(0.5));
	// t.start();
	// std::string culoglikes("loglikescuda.txt");
	// out_num_sp_matrix_to_file(data_sqs, culoglikes);
	// t.end();
	// KALDI_LOG << "write cuda loglikes to file time: " << t.used_time() << "ms";
	
	// t.start();
	// std::string out_inv("gpu-inv-covars.txt");
	// out_num_sp_matrix_to_file(cu_inv_covars_, out_inv);
	// t.end();
	// KALDI_LOG << "write cuda loglikes to file time: " << t.used_time() << "ms";
	// Vector<BaseFloat> gcon(gconsts_);
	// out_vec_to_file(gcon, "gpu-gconst.txt");
	
	t.start();
	CudaLogLikelihoodsPreselect(gselect, feature, gconsts_, means_invcovars_, data_sqs, cu_inv_covars_, loglikes);
	t.end();
	KALDI_LOG << "compute CudaLogLikelihoodsPreselect time: " << t.used_time() << "ms";
	// t.start();
	// std::string culoglikes("loglikescuda.txt");
	// out_mat_to_file(loglikes, culoglikes);
	// t.end();
	// KALDI_LOG << "write cuda loglikes to file time: " << t.used_time() << "ms";
}


void CudaFGMM::ComputePosterior(CuMatrixBase<BaseFloat> *loglikes, BaseFloat min_post)
{
	CudaComputePosterior(loglikes, min_post);
}
