#include "mylib/cpu-compute.h"

bool CpuCompute::Compute(InitSRE *ubm, std::string filename,  std::string ivector_path, int valid_frames)
{
	my_time t;
	t.start();
	SRE<WAVGetData> sre;
	sre.set_filename(filename);
	if(!sre.data_read())
		return false;
	if(!sre.webrtc_vad())
		return false;
	if(!sre.compute_mfcc())
		return false;
	if(!sre.compute_vad())
		return false;
	if(!sre.compute_plp())
		return false;
	if(!sre.add_feats())
		return false;
	if(static_cast<int>(sre.get_feature().NumRows()) < valid_frames)
		return false;
	KALDI_LOG << "valid frames:" << sre.get_feature().NumRows();
	// out_mat_to_file(sre.get_feature(), "cpu-feature.txt");
	if(!sre.fgmm_to_gselect_posterior(*(ubm->ubm)))
		return false;
	if(!sre.extract_ivector(*(ubm->ubm)))
		return false;
	out_vec_to_file(sre.get_ivector(), ivector_path);
	t.end();
	KALDI_LOG << "total compute time: " << t.used_time() << "ms";
	return true;
}


