#ifndef PCM_DATA
#define PCM_DATA

#include "mylib/pcm-reader.h"
#include "matrix/matrix-lib.h"
#include "base/kaldi-common.h"
#include "util/common-utils.h"
#include "util/kaldi-io.h"
#include <fcntl.h>

namespace kaldi
{

class PCMGetData
{
	public:
		PCMGetData() {}
		~PCMGetData() {}
		bool Read(std::string filename, int32 channel, BaseFloat samp_freq);
		Vector<BaseFloat> getData() { return voice_data_;}

	private:
		Vector<BaseFloat> voice_data_;
};

}
#endif
