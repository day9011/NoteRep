#ifndef KALDI_FEAT_PCM_READER_H_
#define KALDI_FEAT_PCM_READER_H_

#include <cstring>

#include "base/kaldi-types.h"
#include "matrix/kaldi-vector.h"
#include "matrix/kaldi-matrix.h"
#include "mylib/conf.h"


namespace kaldi
{


class PCMData {
 public:

  PCMData(BaseFloat samp_freq, const MatrixBase<BaseFloat> &data)
      : data_(data), samp_freq_(samp_freq) {}

  PCMData() : samp_freq_(0.0) {}

  void Read(std::istream &is, long fileLen);

  void Write(std::ostream &os) const;

  const Matrix<BaseFloat> &Data() const { return data_; }

  BaseFloat SampFreq() const { return samp_freq_; }
  
  // Returns the duration in seconds
  BaseFloat Duration() const { return data_.NumCols()/samp_freq_; }

  void CopyFrom(const PCMData &other) {
    samp_freq_ = other.samp_freq_;
    data_.CopyFromMat(other.data_);
  }

  void Clear() {
    data_.Resize(0, 0);
    samp_freq_ = 0.0;
  }

 private:
  static const uint32 kBlockSize = 1048576;  // 1024 * 1024, use 1M bytes
  Matrix<BaseFloat> data_;
  BaseFloat samp_freq_;
  static void Expect4ByteTag(std::istream &is, const char *expected);
  uint32 ReadUint32(std::istream &is, bool swap);
  uint16 ReadUint16(std::istream &is, bool swap);
  static void Read4ByteTag(std::istream &is, char *dest);

  static void WriteUint32(std::ostream &os, int32 i);
  static void WriteUint16(std::ostream &os, int16 i);
};




// Holder class for .wav files that enables us to read (but not write)
// .wav files. c.f. util/kaldi-holder.h
class PCMHolder {
 public:
  typedef PCMData T;

  static bool Write(std::ostream &os, bool binary, const T &t) {
    // We don't write the binary-mode header here [always binary].
    KALDI_ASSERT(binary == true
                 && "Wave data can only be written in binary mode.");
    try {
      t.Write(os);  // throws exception on failure.
      return true;
    } catch(const std::exception &e) {
      KALDI_WARN << "Exception caught in PCMHolder object (writing).";
      std::cerr << e.what();
      return false;  // write failure.
    }
  }
  void Copy(const T &t) { t_.CopyFrom(t); }

  static bool IsReadInBinary() { return true; }

  void Clear() { t_.Clear(); }

  const T &Value() { return t_; }

  PCMHolder &operator = (const PCMHolder &other) {
    t_.CopyFrom(other.t_);
    return *this;
  }
  PCMHolder(const PCMHolder &other): t_(other.t_) {}

  PCMHolder() {}

  bool Read(std::istream &is, long fileLen) {
    // We don't look for the binary-mode header here [always binary]
    try {
      t_.Read(is, fileLen);  // throws exception on failure.
      return true;
    } catch(const std::exception &e) {
      KALDI_WARN << "Exception caught in PCMHolder object (reading).";
      std::cerr << e.what();
      return false;  // write failure.
    }
  }
 private:
  T t_;
  long fileLen;
};


}
#endif  // KALDI_FEAT_PCM_READER_H_
