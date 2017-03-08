#include "ubm.h"

bool SREUBM::ReadUBMFile(std::string final_ubm, std::string final_ie, std::string gmm_ubm)
{
	long long start, end;
	start = getSystemTime();
	ReadKaldiObject(final_ie, &ie_);
	end = getSystemTime();
	KALDI_LOG << "Read final.ie file time: " << end - start << "ms";
	start = getSystemTime();
	ReadKaldiObject(final_ubm, &fgmm_);
	end = getSystemTime();
	KALDI_LOG << "Read final.ubm file time: " << end - start << "ms";
	start = getSystemTime();
	ReadKaldiObject(gmm_ubm, &gmm_);
	end = getSystemTime();
	KALDI_LOG << "Read gmm.ubm file time: " << end - start << "ms";
	return true;
}
