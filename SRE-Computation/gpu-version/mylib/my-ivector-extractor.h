#ifndef KALDI_IVECTOR_IVECTOR_EXTRACTOR_H_
#define KALDI_IVECTOR_IVECTOR_EXTRACTOR_H_

#include <vector>
#include "base/kaldi-common.h"
#include "matrix/matrix-lib.h"
#include "gmm/model-common.h"
#include "gmm/diag-gmm.h"
#include "gmm/full-gmm.h"
#include "itf/options-itf.h"
#include "util/common-utils.h"
#include "thread/kaldi-mutex.h"
#include "hmm/posterior.h"
#include "mylib/gettime.h"

namespace kaldi {

struct MyIvectorEstimationOptions {
  double acoustic_weight;
  double max_count;
  MyIvectorEstimationOptions(): acoustic_weight(1.0), max_count(0.0) {}
  void Register(OptionsItf *opts) {
      opts->Register("acoustic-weight", &acoustic_weight,
                     "Weight on part of auxf that involves the data (e.g. 0.2); "
                     "if this weight is small, the prior will have more effect.");
      opts->Register("max-count", &max_count,
                     "Maximum frame count (affects prior scaling): if >0, the prior "
                     "term will be scaled up after the frame count exceeds this "
                     "value.  Note that this count is considered after posterior "
                     "scaling (e.g. --acoustic-weight option, or scale argument to "
                     "scale-post), so you would normally use a cutoff 10 times "
                     "smaller than the corresponding number of frames.");
		    }
};


class MyIvectorExtractor;
class MyIvectorExtractorComputeDerivedVarsClass;

class MyIvectorExtractorUtteranceStats {
 public:
  MyIvectorExtractorUtteranceStats(int32 num_gauss, int32 feat_dim,
                                 bool need_2nd_order_stats):
      gamma_(num_gauss), X_(num_gauss, feat_dim) {
    if (need_2nd_order_stats) {
      S_.resize(num_gauss);
      for (int32 i = 0; i < num_gauss; i++)
        S_[i].Resize(feat_dim);
    }
  }

  void AccStats(const MatrixBase<BaseFloat> &feats,
                const Posterior &post);

  void Scale(double scale); // Used to apply acoustic scale.

  double NumFrames() { return gamma_.Sum(); }

// protected:
  friend class MyIvectorExtractor;
  Vector<double> gamma_; // zeroth-order stats (summed posteriors), dimension [I]
  Matrix<double> X_; // first-order stats, dimension [I][D]
  std::vector<SpMatrix<double> > S_; // 2nd-order stats, dimension [I][D][D], if
                                     // required.
};


struct MyIvectorExtractorOptions {
  int ivector_dim;
  int num_iters;
  bool use_weights;
  MyIvectorExtractorOptions(): ivector_dim(400), num_iters(2),
                             use_weights(true) { }
  void Register(OptionsItf *opts) {
    opts->Register("num-iters", &num_iters, "Number of iterations in "
                   "iVector estimation (>1 needed due to weights)");
    opts->Register("ivector-dim", &ivector_dim, "Dimension of iVector");
    opts->Register("use-weights", &use_weights, "If true, regress the "
                   "log-weights on the iVector");
  }
};


class MyIvectorExtractor {
 public:

  MyIvectorExtractor(): prior_offset_(0.0) { }

  MyIvectorExtractor(
      const MyIvectorExtractorOptions &opts,
      const FullGmm &fgmm);

  void GetIvectorDistribution(
      const MyIvectorExtractorUtteranceStats &utt_stats,
      VectorBase<double> *mean,
      SpMatrix<double> *var) const;

  double PriorOffset() const { return prior_offset_; }


  void GetIvectorDistMean(
      const MyIvectorExtractorUtteranceStats &utt_stats,
      VectorBase<double> *linear,
      SpMatrix<double> *quadratic) const;

  void GetIvectorDistPrior(
      const MyIvectorExtractorUtteranceStats &utt_stats,
      VectorBase<double> *linear,
      SpMatrix<double> *quadratic) const;

  void GetIvectorDistWeight(
      const MyIvectorExtractorUtteranceStats &utt_stats,
      const VectorBase<double> &mean,
      VectorBase<double> *linear,
      SpMatrix<double> *quadratic) const;


  int32 FeatDim() const;
  int32 IvectorDim() const;
  int32 NumGauss() const;
  bool IvectorDependentWeights() const { return w_.NumRows() != 0; }
  void Write(std::ostream &os, bool binary) const;
  void Read(std::istream &is, bool binary);
  std::vector<Matrix<double> > getSigmaInvM() { return Sigma_inv_M_; }

//protected:
  void ComputeDerivedVars();
  void ComputeDerivedVars(int32 i);
  friend class MyIvectorExtractorComputeDerivedVarsClass;

  void TransformIvectors(const MatrixBase<double> &T,
                         double new_prior_offset);


  Matrix<double> w_;

  Vector<double> w_vec_;

  std::vector<Matrix<double> > M_;

  std::vector<SpMatrix<double> > Sigma_inv_;

  double prior_offset_;

  Vector<double> gconsts_;

  Matrix<double> U_;

  std::vector<Matrix<double> > Sigma_inv_M_;
 private:
  static void InvertWithFlooring(const SpMatrix<double> &quadratic_term,
                                 SpMatrix<double> *var);
};



}  // namespace kaldi


#endif

