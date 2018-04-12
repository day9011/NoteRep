#include "cudalib/my-cuda-mfcc.h"
using namespace std;

namespace kaldi {

CuMfcc::CuMfcc(const MfccOptions &opts)
    : opts_(opts), feature_window_function_(opts.frame_opts) {
	// Step 1: computing dct matrix
	int num_bins = opts.mel_opts.num_bins;
  	int num_ceps = opts.num_ceps;
	KALDI_ASSERT(num_bins != 0 && num_ceps != 0);
	Matrix<BaseFloat> dct_matrix(num_bins, num_bins);
  	Matrix<BaseFloat> dct_matrix_;
	ComputeDctMatrix(&dct_matrix);
  	SubMatrix<BaseFloat> dct_rows(dct_matrix, 0, num_ceps, 0, num_bins);
  	dct_matrix_.Resize(num_ceps, num_bins);
	dct_matrix_.CopyFromMat(dct_rows);  // subset
	dct_matrix_.Transpose();
	cu_dct_matrix_.Resize(dct_matrix_.NumRows(), dct_matrix_.NumCols());	
	cu_dct_matrix_.CopyFromMat(dct_matrix_);
	
  // Step 2: Calculating lifter coefficient
  if (opts.cepstral_lifter != 0.0) {
    Vector<BaseFloat> lifter_coeffs_(opts.num_ceps);
    ComputeLifterCoeffs(opts.cepstral_lifter, &lifter_coeffs_);
  	cu_lifter_coeffs_.Resize(opts.num_ceps);
  	cu_lifter_coeffs_ = lifter_coeffs_;
  }
  // Step 3: Preparing floor energy
  if (opts.energy_floor > 0.0)
    log_energy_floor_ = log(opts.energy_floor);
  // Step 4: Preparing mel banks
  cu_mel_banks_ = new CuMelBanks<BaseFloat>(opts.mel_opts, opts.frame_opts, 1.0);
  
}

CuMfcc::~CuMfcc() {
  if (cu_mel_banks_ != NULL)
    delete cu_mel_banks_;
}

void CuMfcc::ExtractWindow(CuMatrixBase<BaseFloat> &cu_frames_){
	KALDI_ASSERT(cu_frames_.NumRows() != 0 && cu_frames_.NumCols() != 0);
	if (opts_.frame_opts.dither != 0.0){
		CudaDither(cu_frames_, opts_.frame_opts.dither);	
		CudaSetZero(cu_frames_);	
	}
	if(opts_.frame_opts.remove_dc_offset){
		CuVector<BaseFloat> wave_sum_(cu_frames_.NumRows());
		CudaWaveSum(cu_frames_, wave_sum_);
		CudaDcOffset(cu_frames_, wave_sum_);	
	}
	log_energy_.Resize(cu_frames_.NumRows());
	CudaLogEnergy(cu_frames_, log_energy_);
	
	if(opts_.frame_opts.preemph_coeff != 0.0)
		CudaPreemp(cu_frames_, opts_.frame_opts.preemph_coeff);
			
	CuVector<BaseFloat> cuwindow(feature_window_function_.window);	
	CudaWaveMul(cu_frames_, cuwindow);
}

void CuMfcc::DCT(CuMatrix<BaseFloat> &feature){
	if(feature.NumRows() != cu_mel_energy_.NumRows() || feature.NumCols() != cu_mel_energy_.NumCols())
		feature.Resize(cu_mel_energy_.NumRows(), cu_dct_matrix_.NumCols());
	feature.AddMatMat(static_cast<float>(1.0), cu_mel_energy_, kNoTrans, cu_dct_matrix_, kNoTrans, static_cast<float>(0.0));
	if (opts_.cepstral_lifter != 0.0)
		CudaWaveMul(feature, cu_lifter_coeffs_);
	if (opts_.use_energy) {
		CudaSetEnergy(feature, log_energy_, opts_.energy_floor, log_energy_floor_);
	}

}

void CuMfcc::Compute(const CuVectorBase<BaseFloat> &wave,
                   CuMatrix<BaseFloat> &output){
  KALDI_ASSERT(cu_mel_banks_ != NULL);
  ComputeInternal(wave, *cu_mel_banks_, output);  
}


void CuMfcc::ComputeInternal(const CuVectorBase<BaseFloat> &wave,
                           const CuMelBanks<BaseFloat> &mel_banks,
                           CuMatrix<BaseFloat> &output){
	// Step 1: extract wave into frames
	int num_frames = NumFrames(wave.Dim(), opts_.frame_opts);
	cu_frames_.Resize(num_frames, opts_.frame_opts.WindowSize());
	CudaSetZero(cu_frames_);	
	CudaVecMemcpy(cu_frames_, wave, -80);
	
	// Step 2: extract window from frames
	this->ExtractWindow(cu_frames_);
	
	// Step 3: perform srfft
	CudaSrfft(cu_frames_);
	
	// Step 4: compute mel energy
	CudaComputePower(cu_frames_);
	mel_banks.Compute(cu_frames_, cu_mel_energy_);
	cu_mel_energy_.ApplyFloor(std::numeric_limits<BaseFloat>::min());
	cu_mel_energy_.ApplyLog();

	// Step 5: perform dct
	DCT(output);
}






}  // namespace kaldi
