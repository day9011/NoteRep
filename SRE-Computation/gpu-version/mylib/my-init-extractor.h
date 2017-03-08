#ifndef MY_INIT_IVECTOR_EXTRACTOR_H_
#define MY_INIT_IVECTOR_EXTRACTOR_H_

#include <vector>
#include "ivector/ivector-extractor.h"
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


class InitIvectorExtractor;

class InitIvectorExtractor {
 public:

  InitIvectorExtractor(): prior_offset_(0.0) { }

  InitIvectorExtractor(
      const IvectorExtractorOptions &opts,
      const FullGmm &fgmm);

  double PriorOffset() const { return prior_offset_; }

  int32 FeatDim() const;
  int32 IvectorDim() const;
  int32 NumGauss() const;
  bool IvectorDependentWeights() const { return w_.NumRows() != 0; }
  void Write(std::ostream &os, bool binary) const;
  void Read(std::istream &is, bool binary);

  // Note: we allow the default assignment and copy operators
  // because they do what we want.
//protected:
  void ComputeDerivedVars();
  void ComputeDerivedVars(int32 i);
  friend class InitIvectorExtractorComputeDerivedVarsClass;

  // Imagine we'll project the iVectors with transformation T, so apply T^{-1}
  // where necessary to keep the model equivalent.  Used to keep unit variance
  // (like prior re-estimation).
  void TransformIvectors(const MatrixBase<double> &T,
                         double new_prior_offset);


  /// Weight projection vectors, if used.  Dimension is [I][S]
  Matrix<double> w_;

  /// If we are not using weight-projection vectors, stores the Gaussian mixture
  /// weights from the UBM.  This does not affect the iVector; it is only useful
  /// as a way of making sure the log-probs are comparable between systems with
  /// and without weight projection matrices.
  Vector<double> w_vec_;

  /// Ivector-subspace projection matrices, dimension is [I][D][S].
  /// The I'th matrix projects from ivector-space to Gaussian mean.
  /// There is no mean offset to add-- we deal with it by having
  /// a prior with a nonzero mean.
  std::vector<Matrix<double> > M_;

  /// Inverse variances of speaker-adapted model, dimension [I][D][D].
  std::vector<SpMatrix<double> > Sigma_inv_;

  /// 1st dim of the prior over the ivector has an offset, so it is not zero.
  /// This is used to handle the global offset of the speaker-adapted means in a
  /// simple way.
  double prior_offset_;

  // Below are *derived variables* that can be computed from the
  // variables above.

  /// The constant term in the log-likelihood of each Gaussian (not
  /// counting any weight).
  Vector<double> gconsts_;

  /// U_i = M_i^T \Sigma_i^{-1} M_i is a quantity that comes up
  /// in ivector estimation.  This is conceptually a
  /// std::vector<SpMatrix<double> >, but we store the packed-data
  /// in the rows of a matrix, which gives us an efficiency
  /// improvement (we can use matrix-multiplies).
  Matrix<double> U_;

  /// The product of Sigma_inv_[i] with M_[i].
  std::vector<Matrix<double> > Sigma_inv_M_;
 private:
  // var <-- quadratic_term^{-1}, but done carefully, first flooring eigenvalues
  // of quadratic_term to 1.0, which mathematically is the least they can be,
  // due to the prior term.
  static void InvertWithFlooring(const SpMatrix<double> &quadratic_term,
                                 SpMatrix<double> *var);
};


}  // namespace kaldi


#endif

