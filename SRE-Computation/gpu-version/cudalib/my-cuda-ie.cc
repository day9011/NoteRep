#include "cudalib/my-cuda-ie.h"


//CudaIE Function
int32 CudaIE::FeatDim() const
{
	KALDI_ASSERT(!M_.empty());
	return M_[0].NumRows();
}

int32 CudaIE::IvectorDim() const
{
	if (M_.empty())
		return 0.0;
	else
		return M_[0].NumCols();
}

int32 CudaIE::NumGauss() const
{
	return static_cast<int32>(M_.size());
}

void CudaIE::GetIvectorDistribution(const CudaIEStats &utt_stats,
									CuVectorBase<double> *mean) const
{
	my_time t;
	CuVector<double> linear(IvectorDim());
	CuSpMatrix<double> quadratic(IvectorDim());
	GetIvectorDistMean(utt_stats, &linear, &quadratic);
	t.start();
	GetIvectorDistPrior(utt_stats, &linear, &quadratic);
	t.end();
	KALDI_LOG << "get ivector dist prior time: " << t.used_time() << "ms";
	t.start();
	quadratic.Invert();
	t.end();
	KALDI_LOG << "compute quadratic invert time: " << t.used_time() << "ms";
	t.start();
	mean->AddSpVec(1.0, quadratic, linear, 0.0);
	t.end();
	KALDI_LOG << "compute AddSpVec time: " << t.used_time() << "ms";
}

void CudaIE::GetIvectorDistMean(const CudaIEStats &utt_stats,
						CuVectorBase<double> *linear,
						CuSpMatrix<double> *quadratic) const
{
	my_time t;
	t.start();
    CudaAddMatVec(CuNum_Sigma_inv_M_, utt_stats.L_X_, linear);	
	t.end();
	KALDI_LOG << "get dist mean parallel time: " << t.used_time() << "ms";
	int32 dim = static_cast<int32>(IvectorDim());
	CuSubVector<double> q_vec(quadratic->Data(), (dim * (dim + 1)) / 2);
	t.start();
	q_vec.AddMatVec(1.0, U_, kTrans, utt_stats.gamma_, 1.0);
	t.end();
	KALDI_LOG << "compute add MatVec time: " << t.used_time() << "ms";
}

void CudaIE::GetIvectorDistPrior(const CudaIEStats &utt_stats, CuVectorBase<double> *linear,
						CuSpMatrix<double> *quadratic) const
{
	(*linear)(0) += prior_offset_;
	quadratic->AddToDiag(1.0);
}

void CudaIE::Read(std::istream &is, bool binary)
{
#if HAVE_CUDA == 1
	ExpectToken(is, binary, "<IvectorExtractor>");
	ExpectToken(is, binary, "<w>");
	w_.Read(is, binary);
	ExpectToken(is, binary, "<w_vec>");
	w_vec_.Read(is, binary);
	ExpectToken(is, binary, "<M>");
	int32 size;
	ReadBasicType(is, binary, &size);
	KALDI_ASSERT(size > 0);
	M_.resize(size);
	for (int32 i = 0; i < size; i++)
		M_[i].Read(is, binary);
	ExpectToken(is, binary, "<SigmaInv>");
	Sigma_inv_.resize(size);
	for (int32 i = 0; i < size; i++)
		Sigma_inv_[i].Read(is, binary);
	ExpectToken(is, binary, "<SigmaInvM>");
	Sigma_inv_M_.resize(size);
	for (int32 i = 0; i < size; i++)
		Sigma_inv_M_[i].Read(is, binary);
	CuNum_Sigma_inv_M_.Resize(size, Sigma_inv_M_[0].NumRows(), Sigma_inv_M_[0].NumCols());
	CuNum_Sigma_inv_M_.CopyFromMats(Sigma_inv_M_);
	ExpectToken(is, binary, "<U>");
	U_.Read(is, binary);
	ExpectToken(is, binary, "<IvectorOffset>");
	ReadBasicType(is, binary, &prior_offset_);
	ExpectToken(is, binary, "</IvectorExtractor>");
#else
	KADLI_WARN << "No Cuda";
	exit(-1);
#endif
}

//CudaIEStats Function
void CudaIEStats::Resize(int32 num_gauss, int32 feat_dim)
{
	gamma_.Resize(num_gauss);
	X_.Resize(num_gauss, feat_dim);
	L_X_.Resize(num_gauss, feat_dim);
}

void CudaIEStats::AccStats(const MatrixBase<BaseFloat> &feats, const Posterior &post)
{
	typedef std::vector<std::pair<int32, BaseFloat> > VecType;
	int32 num_frames = feats.NumRows(),
		  num_gauss = X_.NumRows(),
		  feat_dim = feats.NumCols();
	KALDI_ASSERT(X_.NumCols() == feat_dim);
	KALDI_ASSERT(num_frames == static_cast<int32>(post.size()));
	Vector<double> gamma(num_gauss);
	Matrix<double> X(num_gauss, feat_dim);
	for (int32 t = 0; t < num_frames; t++)
	{
		SubVector<BaseFloat> frame(feats, t);
		const VecType &this_post(post[t]);
		for (VecType::const_iterator iter = this_post.begin();
			 iter != this_post.end(); ++iter)
		{
			int32 i = iter->first;
			KALDI_ASSERT( i >= 0 && i < num_gauss);
			double weight = iter->second;
			gamma(i) += weight;
			X.Row(i).AddVec(weight, frame);
		}
	}
	my_time t;
	t.start();
	gamma_.CopyFromVec(gamma);
	out_vec_to_file(gamma, "cpu-gamma.txt");	
	X_.CopyFromMat(X);
	out_mat_to_file(X, "cpu-X.txt");
	L_X_.CopyFromMat(X_);
	t.end();
	KALDI_LOG << "write cpu uttstat to file time: " << t.used_time() << "ms";
}

void CudaIEStats::AccStats(const CuMatrixBase<BaseFloat> &feats, const CuMatrixBase<BaseFloat> &loglikes)
{
	int32 num_frames = feats.NumRows(),
		  feat_dim = feats.NumCols();
	KALDI_ASSERT(X_.NumCols() == feat_dim);
	KALDI_ASSERT(num_frames == loglikes.NumRows());
	
	my_time t;
	t.start();
	CuMatrix<double> loglikes_tmp(loglikes);
	CuMatrix<double> feats_tmp(feats);
	CudaAddMatColsToVec(loglikes_tmp, &gamma_);
	X_.AddMatMat(1.0, loglikes_tmp, kTrans, feats_tmp, kNoTrans, 0.0);
	L_X_.CopyFromMat(X_);
	// out_mat_to_file(X_, "gpu-X.txt");
	// out_vec_to_file(gamma_, "gpu-gamma.txt");
	t.end();
	KALDI_LOG << "compute uttstat time: " << t.used_time() << "ms";
}


