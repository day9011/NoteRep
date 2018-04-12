#ifndef MY_CUDA_SRE
#define MY_CUDA_SRE
#include <cstdlib>
#include "mylib/sre.h"
#include "feat/feature-functions.h"
#include "cudalib/my-cuda-fgmm.h"
#include "cudalib/my-cuda-gmm.h"
#include "cudalib/my-cuda-ie.h"
#include "cudamatrix/cu-matrix.h"
#include "cudamatrix/cu-vector.h"
#include "cudamatrix/cu-device.h"
#include "cudamatrix/cu-sp-matrix.h"
#include "cudamatrix/cu-tp-matrix.h"
#include "cudamatrix/cu-packed-matrix.h"
#include "util/common-utils.h"
#include "base/kaldi-common.h"
#include "cudamatrix/cu-matrix-lib.h"
#include "mylib/gettime.h"
#include "mylib/option.h"
#include "mylib/conf.h"
#include "cudalib/my-cuda-data-struct.h"
#include "cudalib/my-cuda-tool.h"
#include "cudalib/my-cuda-init.h"
#include "cudalib/my-cuda-mel-computations.h"
#include "cudalib/my-cuda-function.h"
#include "cudalib/my-cuda-mfcc.h"

namespace kaldi
{


template <class DataType>
class CudaSRE: public SRE<DataType>
{
	public:
		CudaSRE(): SRE<DataType>() {}
		~CudaSRE() {}

		bool setVoiceFileName(std::string filename);
		CuMatrix<BaseFloat> get_cuda_feature();
		int32 NumFrames() const { return cu_feature_.NumRows(); }
		Vector<double> get_ivector();
		bool cuda_data_read(int32 channel, BaseFloat samp_freq); // read data in to cuda_matrix	
		bool cuda_voice2frame(); // change wave into frames
		void cuda_ExtractWindow(); // perform functions in ExtractWindow
		bool cuda_srfft(); // distributed version for fft
		bool cuda_mel_energy(); // compute mel energy for each frame
		bool cuda_dct(); // compute dct of the mel energy
		bool cuda_compute_vad(VadEnergyOptions vad_opts); // compute vad energy
		bool cuda_add_feats(SlidingWindowCmnOptions slid_opts, DeltaFeaturesOptions deltas_opts); // compute deltas
		bool cuda_compute_mfcc();
		bool gmm_select(CudaGMM &cu_gmm_);
		bool cuda_compute_posterior(FullGmm &fgmm_);
		bool cuda_compute_posterior(CudaFGMM &cufgmm_);
		bool cuda_compute_ivector(CudaIE &ie);
		const CuMelBanks<BaseFloat> *GetMelBanks(BaseFloat vtln_warp, bool *must_delete) const;
	private:
		CuVector<BaseFloat> cu_voice_data_;
		MyOption opts;
		CuMatrix<BaseFloat> cu_feature_;
		Matrix<BaseFloat> tmp_feature_;
		Vector<BaseFloat> vad_result_;
		CuMatrixInt cu_gselect_;
		Posterior cu_post_;
		CuMatrix<BaseFloat> loglikes_;
		CudaIEStats cu_stats_;
		CuVector<double> ivector_;
		Vector<double> ivector_result_;
		CuVector<BaseFloat> wave_sum_;
		CuVector<BaseFloat> log_energy_;
};

}
#endif

