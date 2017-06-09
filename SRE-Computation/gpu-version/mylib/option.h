#ifndef CPU_OPTION_H
#define CPU_OPTION_H

#include "feat/feature-mfcc.h"
#include "feat/feature-functions.h"
#include "ivector/voice-activity-detection.h"
#include "mylib/conf.h"
#include "mylib/my-sre-option.h"
#include "ivector/ivector-extractor.h"
#include "feat/feature-plp.h"

using namespace kaldi;

struct MyOption
{
    MfccOptions mfcc_opts;
    VadEnergyOptions vad_opts;
    SlidingWindowCmnOptions slid_opts;
    DeltaFeaturesOptions delta_opts;
    SRECustomOption custom_opts;
	IvectorEstimationOptions extractor_opts;
	PlpOptions plp_opts;

    MyOption(): mfcc_opts(), vad_opts(), slid_opts(), delta_opts(), custom_opts(), extractor_opts()
    {
	        vad_opts.vad_energy_threshold = vadEnergyThreshold;
	        vad_opts.vad_energy_mean_scale = vadEnergyMeanScale;
	        mfcc_opts.frame_opts.samp_freq = sampFreq;
	        mfcc_opts.frame_opts.frame_length_ms = frameLength;
			mfcc_opts.frame_opts.snip_edges = useEdges;
	        mfcc_opts.mel_opts.high_freq = highFreq;
	        mfcc_opts.mel_opts.low_freq = lowFreq;
	        mfcc_opts.num_ceps = numCeps;
			mfcc_opts.frame_opts.dither = ditherVal;
	        slid_opts.normalize_variance = normVars;
	        slid_opts.center = cmnCenter;
	        slid_opts.cmn_window = cmnWindow;
	        delta_opts.order = deltaOrder;
	        delta_opts.window = deltaWindow;
			plp_opts.frame_opts.samp_freq = sampFreq;
			plp_opts.frame_opts.frame_length_ms = frameLength;
			plp_opts.frame_opts.snip_edges = useEdges;
	        custom_opts.num_ignore_frames = numIgnoreFrames;
			custom_opts.channel = Channel;
			custom_opts.num_gselect = numGselect;
			custom_opts.min_post = minPosterior;
			custom_opts.posterior_scale = PosteriorScale;
	}

	void Register(OptionsItf *opts)
	{
		mfcc_opts.Register(opts);
		vad_opts.Register(opts);
		slid_opts.Register(opts);
		delta_opts.Register(opts);
		custom_opts.Register(opts);
		extractor_opts.Register(opts);
	}
};

#endif
