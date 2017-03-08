#include <vector>

#include "mylib/my-ivector-extractor.h"
#include "thread/kaldi-task-sequence.h"

namespace kaldi {

int32 MyIvectorExtractor::FeatDim() const {
  KALDI_ASSERT(!M_.empty());
  return M_[0].NumRows();
}

int32 MyIvectorExtractor::IvectorDim() const {
  if (M_.empty()) { return 0.0; }
  else { return M_[0].NumCols(); }
}

int32 MyIvectorExtractor::NumGauss() const {
  return static_cast<int32>(M_.size());
}


// This function basically inverts the input and puts it in the output, but it's
// smart numerically.  It uses the prior knowledge that the "inverse_floor" can
// have no eigenvalues less than one, so it applies that floor (in double
// precision) before inverting.  This avoids certain numerical problems that can
// otherwise occur.
// static
void MyIvectorExtractor::InvertWithFlooring(const SpMatrix<double> &inverse_var,
                                          SpMatrix<double> *var) {
  SpMatrix<double> dbl_var(inverse_var);
  int32 dim = inverse_var.NumRows();
  Vector<double> s(dim);
  Matrix<double> P(dim, dim);
  // Solve the symmetric eigenvalue problem, inverse_var = P diag(s) P^T.
  inverse_var.Eig(&s, &P);
  s.ApplyFloor(1.0);
  s.InvertElements();
  var->AddMat2Vec(1.0, P, kNoTrans, s, 0.0);
}


void MyIvectorExtractor::GetIvectorDistribution(
    const MyIvectorExtractorUtteranceStats &utt_stats,
    VectorBase<double> *mean,
    SpMatrix<double> *var) const {
  if (!IvectorDependentWeights()) {
  	KALDI_LOG << "ivector not depend on weights.";
    Vector<double> linear(IvectorDim());
    SpMatrix<double> quadratic(IvectorDim());
    GetIvectorDistMean(utt_stats, &linear, &quadratic);
    GetIvectorDistPrior(utt_stats, &linear, &quadratic);
    if (var != NULL) {
      var->CopyFromSp(quadratic);
      var->Invert(); // now it's a variance.

      // mean of distribution = quadratic^{-1} * linear...
      mean->AddSpVec(1.0, *var, linear, 0.0);
    } else {
//	  KALDI_LOG << "quadratic rows:" << quadratic.NumRows() << "\tquadratic cols:" << quadratic.NumCols();
//	  KALDI_LOG << "linear dims:" << linear.Dim();
      quadratic.Invert();
      mean->AddSpVec(1.0, quadratic, linear, 0.0);
    }
  } else {
    Vector<double> linear(IvectorDim());
    SpMatrix<double> quadratic(IvectorDim());
    GetIvectorDistMean(utt_stats, &linear, &quadratic);
    GetIvectorDistPrior(utt_stats, &linear, &quadratic);
    // At this point, "linear" and "quadratic" contain
    // the mean and prior-related terms, and we avoid
    // recomputing those.

    Vector<double> cur_mean(IvectorDim());

    SpMatrix<double> quadratic_inv(IvectorDim());
    InvertWithFlooring(quadratic, &quadratic_inv);
    cur_mean.AddSpVec(1.0, quadratic_inv, linear, 0.0);

    KALDI_VLOG(3) << "Trace of quadratic is " << quadratic.Trace()
                  << ", condition is " << quadratic.Cond();
    KALDI_VLOG(3) << "Trace of quadratic_inv is " << quadratic_inv.Trace()
                  << ", condition is " << quadratic_inv.Cond();

    // The loop is finding successively better approximation points
    // for the quadratic expansion of the weights.
    int32 num_iters = 4;
    double change_threshold = 0.1; // If the iVector changes by less than
    // this (in 2-norm), we abort early.
    for (int32 iter = 0; iter < num_iters; iter++) {
      if (GetVerboseLevel() >= 3) {
        int32 show_dim = 5;
        if (show_dim > cur_mean.Dim()) show_dim = cur_mean.Dim();
        KALDI_VLOG(3) << "Current distribution mean is "
                      << cur_mean.Range(0, show_dim) << "... "
                      << ", var trace is " << quadratic_inv.Trace();
      }
      Vector<double> this_linear(linear);
      SpMatrix<double> this_quadratic(quadratic);
      GetIvectorDistWeight(utt_stats, cur_mean,
                           &this_linear, &this_quadratic);
      InvertWithFlooring(this_quadratic, &quadratic_inv);
      Vector<double> mean_diff(cur_mean);
      cur_mean.AddSpVec(1.0, quadratic_inv, this_linear, 0.0);
      mean_diff.AddVec(-1.0, cur_mean);
      double change = mean_diff.Norm(2.0);
      KALDI_VLOG(2) << "On iter " << iter << ", iVector changed by " << change;
      if (change < change_threshold)
        break;
    }
    mean->CopyFromVec(cur_mean);
    if (var != NULL)
      var->CopyFromSp(quadratic_inv);
  }
}


MyIvectorExtractor::MyIvectorExtractor(
    const MyIvectorExtractorOptions &opts,
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

class MyIvectorExtractorComputeDerivedVarsClass {
 public:
  MyIvectorExtractorComputeDerivedVarsClass(MyIvectorExtractor *extractor,
                                          int32 i):
      extractor_(extractor), i_(i) { }
  void operator () () { extractor_->ComputeDerivedVars(i_); }
 private:
  MyIvectorExtractor *extractor_;
  int32 i_;
};

void MyIvectorExtractor::ComputeDerivedVars() {
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
    TaskSequencer<MyIvectorExtractorComputeDerivedVarsClass> sequencer(
        sequencer_opts);
    for (int32 i = 0; i < NumGauss(); i++)
      sequencer.Run(new MyIvectorExtractorComputeDerivedVarsClass(this, i));
  }
  KALDI_LOG << "Done.";
}


void MyIvectorExtractor::ComputeDerivedVars(int32 i) {
  SpMatrix<double> temp_U(IvectorDim());
  // temp_U = M_i^T Sigma_i^{-1} M_i
  temp_U.AddMat2Sp(1.0, M_[i], kTrans, Sigma_inv_[i], 0.0);
  SubVector<double> temp_U_vec(temp_U.Data(),
                               IvectorDim() * (IvectorDim() + 1) / 2);
  U_.Row(i).CopyFromVec(temp_U_vec);

  Sigma_inv_M_[i].Resize(FeatDim(), IvectorDim());
  Sigma_inv_M_[i].AddSpMat(1.0, Sigma_inv_[i], M_[i], kNoTrans, 0.0);
}


void MyIvectorExtractor::GetIvectorDistWeight(
    const MyIvectorExtractorUtteranceStats &utt_stats,
    const VectorBase<double> &mean,
    VectorBase<double> *linear,
    SpMatrix<double> *quadratic) const {
  // If there is no w_, then weights do not depend on the iVector
  // and the weights contribute nothing to the distribution.
  if (!IvectorDependentWeights())
    return;

  Vector<double> logw_unnorm(NumGauss());
  logw_unnorm.AddMatVec(1.0, w_, kNoTrans, mean, 0.0);

  Vector<double> w(logw_unnorm);
  w.ApplySoftMax(); // now w is the weights.

  // See eq.58 in SGMM paper
  // http://www.sciencedirect.com/science/article/pii/S088523081000063X
  // linear_coeff(i) = \gamma_{jmi} - \gamma_{jm} \hat{w}_{jmi} + \max(\gamma_{jmi}, \gamma_{jm} \hat{w}_{jmi} \hat{\w}_i \v_{jm}
  // here \v_{jm} corresponds to the iVector.  Ignore the j,m indices.
  Vector<double> linear_coeff(NumGauss());
  Vector<double> quadratic_coeff(NumGauss());
  double gamma = utt_stats.gamma_.Sum();
  for (int32 i = 0; i < NumGauss(); i++) {
    double gamma_i = utt_stats.gamma_(i);
    double max_term = std::max(gamma_i, gamma * w(i));
    linear_coeff(i) = gamma_i - gamma * w(i) + max_term * logw_unnorm(i);
    quadratic_coeff(i) = max_term;
  }
  linear->AddMatVec(1.0, w_, kTrans, linear_coeff, 1.0);

  // *quadratic += \sum_i quadratic_coeff(i) w_i w_i^T, where w_i is
  //    i'th row of w_.
  quadratic->AddMat2Vec(1.0, w_, kTrans, quadratic_coeff, 1.0);
}

void MyIvectorExtractor::GetIvectorDistMean(
    const MyIvectorExtractorUtteranceStats &utt_stats,
    VectorBase<double> *linear,
    SpMatrix<double> *quadratic) const {
  int32 I = NumGauss();
  for (int32 i = 0; i < I; i++) {
    double gamma = utt_stats.gamma_(i);
    if (gamma != 0.0) {
      SubVector<double> x(utt_stats.X_, i); // == \gamma(i) \m_i
      // next line: a += \gamma_i \M_i^T \Sigma_i^{-1} \m_i
      linear->AddMatVec(1.0, Sigma_inv_M_[i], kTrans, x, 1.0);
    }
  }
  SubVector<double> q_vec(quadratic->Data(), IvectorDim()*(IvectorDim()+1)/2);
  q_vec.AddMatVec(1.0, U_, kTrans, utt_stats.gamma_, 1.0);
}

void MyIvectorExtractor::GetIvectorDistPrior(
    const MyIvectorExtractorUtteranceStats &utt_stats,
    VectorBase<double> *linear,
    SpMatrix<double> *quadratic) const {

  (*linear)(0) += prior_offset_; // the zero'th dimension has an offset mean.
  /// The inverse-variance for the prior is the unit matrix.
  quadratic->AddToDiag(1.0);
}



void MyIvectorExtractor::TransformIvectors(const MatrixBase<double> &T,
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


void MyIvectorExtractor::Write(std::ostream &os, bool binary) const {
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
  WriteToken(os, binary, "<IvectorOffset>");
  WriteBasicType(os, binary, prior_offset_);
  WriteToken(os, binary, "</IvectorExtractor>");
}


void MyIvectorExtractor::Read(std::istream &is, bool binary) {
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
  ExpectToken(is, binary, "<SigmaInvM>");
  Sigma_inv_M_.resize(size);
  for (int32 i = 0; i < size; i++)
    Sigma_inv_M_[i].Read(is, binary);
  ExpectToken(is, binary, "<U>");
  U_.Read(is, binary);
  ExpectToken(is, binary, "<IvectorOffset>");
  ReadBasicType(is, binary, &prior_offset_);
  ExpectToken(is, binary, "</IvectorExtractor>");
  end = getSystemTime();
  KALDI_LOG << "Read final.ie file time: " << end - start << "ms";
}


void MyIvectorExtractorUtteranceStats::AccStats(
    const MatrixBase<BaseFloat> &feats,
    const Posterior &post) {
  typedef std::vector<std::pair<int32, BaseFloat> > VecType;
  int32 num_frames = feats.NumRows(),
      num_gauss = X_.NumRows(),
      feat_dim = feats.NumCols();
  KALDI_ASSERT(X_.NumCols() == feat_dim);
  KALDI_ASSERT(feats.NumRows() == static_cast<int32>(post.size()));
  bool update_variance = (!S_.empty());
  SpMatrix<double> outer_prod(feat_dim);
  for (int32 t = 0; t < num_frames; t++) {
    SubVector<BaseFloat> frame(feats, t);
    const VecType &this_post(post[t]);
    if (update_variance) {
      outer_prod.SetZero();
      outer_prod.AddVec2(1.0, frame);
    }
    for (VecType::const_iterator iter = this_post.begin();
         iter != this_post.end(); ++iter) {
      int32 i = iter->first; // Gaussian index.
      KALDI_ASSERT(i >= 0 && i < num_gauss &&
                   "Out-of-range Gaussian (mismatched posteriors?)");
      double weight = iter->second;
      gamma_(i) += weight;
      X_.Row(i).AddVec(weight, frame);
      if (update_variance)
        S_[i].AddSp(weight, outer_prod);
    }
  }
}

void MyIvectorExtractorUtteranceStats::Scale(double scale) {
  gamma_.Scale(scale);
  X_.Scale(scale);
  for (size_t i = 0; i < S_.size(); i++)
    S_[i].Scale(scale);
}


} // namespace kaldi
