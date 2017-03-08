#include "mylib/my-init-extractor.h"
#include "thread/kaldi-task-sequence.h"
#include "gmm/am-diag-gmm.h"
#include "util/common-utils.h"
#include "base/kaldi-common.h"

using namespace kaldi;

int main(int argc, char *argv[]) {
	typedef kaldi::int32 int32;
	typedef kaldi::int64 int64;
	const char *usage = "for a test";
	ParseOptions po(usage);
	bool compute_objf_change = true;
	bool binary = true;
	IvectorEstimationOptions ivec_opts;
	TaskSequencerConfig sequencer_config;
	po.Register("compute-objf-change", &compute_objf_change, "compute the change in objective function from using nonzero iVector");
	po.Register("binary", &binary, "output mode(true/false, default:true)");
	ivec_opts.Register(&po);
	sequencer_config.Register(&po);
	po.Read(argc, argv);
	if (po.NumArgs() != 2) {
		po.PrintUsage();
		exit(-1);
	}
	std::string ivector_extractor_filename = po.GetArg(1),
		ivectors_wspecifier = po.GetArg(2);
	g_num_threads = sequencer_config.num_threads;
	InitIvectorExtractor extractor;
	ReadKaldiObject(ivector_extractor_filename, &extractor);
	WriteKaldiObject(extractor, ivectors_wspecifier, binary);
}
