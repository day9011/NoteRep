#ifndef MY_SRE_OPTION
#define MY_SRE_OPTION

#include "base/kaldi-common.h"
#include "util/common-utils.h"
#include "itf/options-itf.h"

namespace kaldi
{


struct SRECustomOption
{
	int32 num_ignore_frames;
	int32 channel;
	int32 num_gselect;
	BaseFloat min_post;
	double posterior_scale;

	SRECustomOption(): num_ignore_frames(500), channel(-1), num_gselect(20), min_post(0.025), posterior_scale(1.0) {}

	void Register(OptionsItf *opts)
	{
		opts->Register("num-ignore-frames", &num_ignore_frames,
					   "The first number of frames will be ignored.");
		opts->Register("channel", &channel,
					   "Channel to extract (-1 -> expect mono, 0 -> left, 1 -> right)\n");
		opts->Register("num-gselect", &num_gselect,
					   "Number of Gaussians to keep per frame\n");
		opts->Register("min-post", &min_post,
					   "min posterior\n");
		opts->Register("posterior-scale", &posterior_scale,
					   "posterior scale coefficient\n");
	}
};

}
#endif
