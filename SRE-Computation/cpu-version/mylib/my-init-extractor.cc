
#include <vector>

#include "mylib/my-init-extractor.h"
#include "thread/kaldi-task-sequence.h"

namespace kaldi {

int32 InitIvectorExtractor::FeatDim() const {
  KALDI_ASSERT(!M_.empty());
  return M_[0].NumRows();
}

int32 InitIvectorExtractor::IvectorDim() const {
  if (M_.empty()) { return 0.0; }
  else { return M_[0].NumCols(); }
}

int32 InitIvectorExtractor::NumGauss() const {
  return static_cast<int32>(M_.size());
}



InitIvectorExtractor::InitIvectorExtractor(
    const IvectorExtractorOptions &opts,
    const FullGmm &fgmm) {
  KALDI_ASSERT(opts.ivector_dim > 0);
  Sigma_inv_.resize(fgmm.NumGauss());
  for (int32 i = 0; i < fgmm.NumGauss(); i++) {
    const SpMatrix<BaseFloat> &inv_var = fgmm.inv_covars()[i];
    Sigma_inv_[i].Resize(inv_var.NumRows());
    Sigma_inv_[i].CopyFromSp(inv_var);
  }
  Matrix<double> gmm_means;
  fgmm.GetMeans(&gmm_means);
  KALDI_ASSERT(!Sigma_inv_.empty());
  int32 feature_dim = Sigma_inv_[0].NumRows(),
      num_gauss = Sigma_inv_.size();

  prior_offset_ = 100.0; // hardwired for now.  Must be nonzero.
  gmm_means.Scale(1.0 / prior_offset_);

  M_.resize(num_gauss);
  for (int32 i = 0; i < num_gauss; i++) {
    M_[i].Resize(feature_dim, opts.ivector_dim);
    M_[i].SetRandn();
    M_[i].CopyColFromVec(gmm_means.Row(i), 0);
  }
  if (opts.use_weights) { // will regress the log-weights on the iVector.
    w_.Resize(num_gauss, opts.ivector_dim);
  } else {
    w_vec_.Resize(fgmm.NumGauss());
    w_vec_.CopyFromVec(fgmm.weights());
  }
  ComputeDerivedVars();
}

class InitIvectorExtractorComputeDerivedVarsClass {
 public:
  InitIvectorExtractorComputeDerivedVarsClass(InitIvectorExtractor *extractor,
                                          int32 i):
      extractor_(extractor), i_(i) { }
  void operator () () { extractor_->ComputeDerivedVars(i_); }
 private:
  InitIvectorExtractor *extractor_;
  int32 i_;
};

void InitIvectorExtractor::ComputeDerivedVars() {
  KALDI_LOG << "Computing derived variables for iVector extractor";
  gconsts_.Resize(NumGauss());
  for (int32 i = 0; i < NumGauss(); i++) {
    double var_logdet = -Sigma_inv_[i].LogPosDefDet();
    gconsts_(i) = -0.5 * (var_logdet + FeatDim() * M_LOG_2PI);
    // the gconsts don't contain any weight-related terms.
  }
  U_.Resize(NumGauss(), IvectorDim() * (IvectorDim() + 1) / 2);
  Sigma_inv_M_.resize(NumGauss());

  // Note, we could have used RunMultiThreaded for this and similar tasks we
  // have here, but we found that we don't get as complete CPU utilization as we
  // could because some tasks finish before others.
  {
    TaskSequencerConfig sequencer_opts;
    sequencer_opts.num_threads = g_num_threads;
    TaskSequencer<InitIvectorExtractorComputeDerivedVarsClass> sequencer(
        sequencer_opts);
    for (int32 i = 0; i < NumGauss(); i++)
      sequencer.Run(new InitIvectorExtractorComputeDerivedVarsClass(this, i));
  }
  KALDI_LOG << "Done.";
}


void InitIvectorExtractor::ComputeDerivedVars(int32 i) {
  SpMatrix<double> temp_U(IvectorDim());
  // temp_U = M_i^T Sigma_i^{-1} M_i
  temp_U.AddMat2Sp(1.0, M_[i], kTrans, Sigma_inv_[i], 0.0);
  SubVector<double> temp_U_vec(temp_U.Data(),
                               IvectorDim() * (IvectorDim() + 1) / 2);
  U_.Row(i).CopyFromVec(temp_U_vec);

  Sigma_inv_M_[i].Resize(FeatDim(), IvectorDim());
  Sigma_inv_M_[i].AddSpMat(1.0, Sigma_inv_[i], M_[i], kNoTrans, 0.0);
}

void InitIvectorExtractor::TransformIvectors(const MatrixBase<double> &T,
                                         double new_prior_offset) {
  Matrix<double> Tinv(T);
  Tinv.Invert();
  // w <-- w Tinv.  (construct temporary copy with Matrix<double>(w))
  if (IvectorDependentWeights())
    w_.AddMatMat(1.0, Matrix<double>(w_), kNoTrans, Tinv, kNoTrans, 0.0);
  // next: M_i <-- M_i Tinv.  (construct temporary copy with Matrix<double>(M_[i]))
  for (int32 i = 0; i < NumGauss(); i++)
    M_[i].AddMatMat(1.0, Matrix<double>(M_[i]), kNoTrans, Tinv, kNoTrans, 0.0);
  KALDI_LOG << "Setting iVector prior offset to " << new_prior_offset;
  prior_offset_ = new_prior_offset;
}



void InitIvectorExtractor::Write(std::ostream &os, bool binary) const {
  long long start, end;
  start = getSystemTime();
  WriteToken(os, binary, "<IvectorExtractor>");
  WriteToken(os, binary, "<w>");
  w_.Write(os, binary);
  WriteToken(os, binary, "<w_vec>");
  w_vec_.Write(os, binary);
  WriteToken(os, binary, "<M>");
  int32 size = M_.size();
  WriteBasicType(os, binary, size);
  for (int32 i = 0; i < size; i++)
    M_[i].Write(os, binary);
  WriteToken(os, binary, "<SigmaInv>");
  KALDI_ASSERT(size == static_cast<int32>(Sigma_inv_.size()));
  for (int32 i = 0; i < size; i++)
    Sigma_inv_[i].Write(os, binary);
  WriteToken(os, binary, "<SigmaInvM>");
  KALDI_ASSERT(size == static_cast<int32>(Sigma_inv_M_.size()));
  for(int32 i = 0; i < size; i++)
  	Sigma_inv_M_[i].Write(os, binary);
  WriteToken(os, binary, "<U>");
  U_.Write(os, binary);
  WriteToken(os, binary, "<IvectorOffset>");
  WriteBasicType(os, binary, prior_offset_);
  WriteToken(os, binary, "</IvectorExtractor>");
  end = getSystemTime();
  KALDI_LOG << "Write to new file time: " << end - start << "ms";
}


void InitIvectorExtractor::Read(std::istream &is, bool binary) {
  long long start, end;
  start = getSystemTime();
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
  ExpectToken(is, binary, "<IvectorOffset>");
  ReadBasicType(is, binary, &prior_offset_);
  ExpectToken(is, binary, "</IvectorExtractor>");
  end = getSystemTime();
  KALDI_LOG << "Read final.ie file time: " << end - start << "ms";
  start = getSystemTime();
  ComputeDerivedVars();
  end = getSystemTime();
  KALDI_LOG << "Compute derived vars time: " << end - start << "ms";
}




} // namespace kaldi
