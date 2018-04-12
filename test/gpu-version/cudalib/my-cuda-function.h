#ifndef MY_CUDA_FUNCTION
#define MY_CUDA_FUNCTION

#include "util/common-utils.h"
#include "cudalib/my-cuda-function-kernel.h"
#include "base/kaldi-error.h"
#include "base/kaldi-common.h"
#include "matrix/matrix-common.h"
#include "matrix/sp-matrix.h"
#include "cudamatrix/cu-common.h"
#include "cudamatrix/cu-device.h"
#include "cudamatrix/cu-vector.h"
#include "cudamatrix/cu-matrix.h"
#include "cudamatrix/cu-tp-matrix.h"
#include "cudamatrix/cu-sp-matrix.h"
#include "cudamatrix/cu-packed-matrix.h"
#include "cudamatrix/cu-matrix-lib.h"
#include "cudalib/my-cuda-data-struct.h"


namespace kaldi
{
template<typename Real>
class CudaFFT {
	public:
		CudaFFT(int32 dim): dim_(dim) {}
		~CudaFFT() {}

		void compute(Real *data);
	private:
		int32 dim_;
};


template<typename Real>
void CudaFFT<Real>::compute(Real *data)
{
	my_cuda_compute_fft(data, dim_);
}

template<typename Real>
inline void CudaGMMSelect(const CuMatrixBase<Real> &loglikesmat, CuMatrixInt *gmm_out, int32_cuda num_gselect)
{
	int32_cuda dimBlock(CU1DBLOCK);
	int32_cuda dimGrid = (loglikesmat.NumRows() + dimBlock - 1) / dimBlock;
	my_cuda_gmm_select(dimGrid, dimBlock, loglikesmat.Data(), loglikesmat.Dim(), num_gselect, gmm_out->Data());
}

template<typename Real>
Real CudaTraceSpSpLower(const CuSpMatrix<Real> &A, const CuSpMatrix<Real> &B)
{
	KALDI_ASSERT(A.NumRows() == B.NumRows());
	if (A.NumRows() == 0) return 0.0;
	MatrixIndexT nr = A.NumRows(), size = nr * (nr + 1) / 2;
	CuSubVector<Real> Aall(A.Data(), size);
	CuSubVector<Real> Ball(B.Data(), size);
	return VecVec(Aall, Ball);
}

template<typename Real>
inline void CudaMatVecVec(const CuMatrixBase<Real> &A, const CuMatrixBase<Real> &B, CuVectorBase<Real> *out)
{
	KALDI_ASSERT(A.NumRows() == B.NumRows() && A.NumCols() == B.NumCols() && A.NumRows() == out->Dim());
	my_cuda_MatVecVec(A.Data(), B.Data(), A.Dim(), B.Dim(), out->Data(), n_blocks(A.NumRows(), CU1DBLOCK), CU1DBLOCK);
}

template<typename Real>
inline void CudaMatVecVec(const CuNumSpMatrix<Real> &A, const CuNumSpMatrix<Real> &B, CuVectorBase<Real> *out)
{
	KALDI_ASSERT(A.NumRows() == B.NumRows() && A.NumCols() == B.NumCols() && A.NumRows() == out->Dim());
	my_cuda_MatVecVec(A.Data(), B.Data(), A.Dim(), B.Dim(), out->Data(), n_blocks(A.NumRows(), CU1DBLOCK), CU1DBLOCK);
}

template<typename Real>
inline void CudaLogLikelihoodsPreselect(const CuMatrixIntBase &gselect, const CuMatrixBase<Real> &feature, const CuVectorBase<Real> &gconsts, const CuMatrixBase<Real> &means_invcovars, const CuNumPackedMatrix<Real> &data_sqs, const CuNumPackedMatrix<Real> &cu_inv_covars_, CuMatrixBase<Real> *loglikes)
{
	KALDI_ASSERT(gselect.NumRows() == feature.NumRows() && loglikes->NumRows() == feature.NumRows() && loglikes->NumCols() == gconsts.Dim());
	dim3 Bl(CU2DBLOCK, CU2DBLOCK);
	dim3 Gr(n_blocks(feature.NumRows(), CU2DBLOCK), n_blocks(gselect.NumCols(), CU2DBLOCK));
	my_cuda_LogLikelihoodsPreselect(gselect.Data(), gselect.NumRows(), gselect.NumCols(), feature.Data(), feature.Dim(), gconsts.Data(), gconsts.Dim(), means_invcovars.Data(), means_invcovars.Dim(), data_sqs.Data(), cu_inv_covars_.Data(), data_sqs.Dim().stride, loglikes->Data(), loglikes->Dim(), Gr, Bl);
}

template<typename Real>
inline void CudaScaleDiagNumSpMat(CuNumPackedMatrix<Real> *mat, Real alpha)
{
	dim3 Bl(CU2DBLOCK, CU2DBLOCK);
	dim3 Gr(n_blocks(mat->NumSizes(), CU2DBLOCK), n_blocks(mat->NumRows(), CU2DBLOCK));
	my_cuda_scale_diag_numsp(mat->Data(), mat->NumSizes(), mat->Dim(), alpha, Gr, Bl);
}

template<typename Real>
inline void CudaComputePosterior(CuMatrixBase<Real> *loglikes, Real min_post)
{
	int32_cuda Bl = CU1DBLOCK;
	int32_cuda Gr = n_blocks(loglikes->NumRows(), CU1DBLOCK);
	my_cuda_compute_posterior(loglikes->Data(), loglikes->Dim(), min_post, Gr, Bl);
}

template<typename Real>
inline void CudaMatApplySoftMax(CuMatrixBase<Real> *loglikes)
{
	int32_cuda Bl = CU1DBLOCK;
	int32_cuda Gr = n_blocks(loglikes->NumRows(), CU1DBLOCK);
	my_cuda_MatApplySoftMax(loglikes->Data(), loglikes->Dim(), Gr, Bl);
}

template<typename Real>
inline void CudaAddMatColsToVec(const CuMatrixBase<Real> &mat, CuVectorBase<Real> *vec)
{
	KALDI_ASSERT(mat.NumCols() == vec->Dim());
	int32_cuda Bl = CU1DBLOCK;
	int32_cuda Gr = n_blocks(vec->Dim(), CU1DBLOCK);
	my_cuda_AddMatColsToVec(mat.Data(), mat.Dim(), vec->Data(), Gr, Bl);
}

template<typename Real>
inline void CudaAddMatVec(const CuNumMatrix<Real> &mat, const CuLinearMatrixBase<Real> &vec, CuVectorBase<Real> *res){
	my_cuda_add_mat_vec(vec.Data(), mat.Data(), res->Data(), vec.NumRows() * vec.NumCols(), vec.NumRows() * vec.NumCols(), mat.NumCols());	
}

template<typename Real>
inline void CudaVecMemcpy(CuMatrix<Real>& des, const CuVectorBase<Real>& src, int32_cuda blank){
	KALDI_ASSERT(blank < des.NumCols());
	my_cuda_vec_memcpy(src.Data(), src.Dim(), blank, des.Data(), des.NumRows(), des.NumCols(), des.Stride(), des.NumRows(), des.NumCols());
}

template<typename Real>
inline void CudaMelPower(const CuMatrixBase<Real> &power_spectrum, const int32_cuda *offset, const int32_cuda *bins_len, const CuMatrixBase<Real> &bins, CuMatrixBase<Real> &mel_energies_out, bool htk_mode){
	KALDI_ASSERT(power_spectrum.NumRows() != 0 && power_spectrum.NumCols() != 0);
	KALDI_ASSERT(offset != NULL);
	KALDI_ASSERT(bins_len != NULL);
	KALDI_ASSERT(bins.NumRows() != 0 && bins.NumCols() != 0);
	KALDI_ASSERT(mel_energies_out.NumRows() != 0 && mel_energies_out.NumCols() != 0);
	my_cuda_parallel_mel_vecvec(power_spectrum.Data(), power_spectrum.NumRows(), power_spectrum.Stride(), offset, bins_len, bins.Data(), bins.NumRows(), bins.NumCols(), mel_energies_out.Data(), mel_energies_out.Stride(), htk_mode? 1:0, power_spectrum.NumRows());
}

template<typename Real>
inline void CudaDither(CuMatrixBase<Real>& waveform, float dither_value){
	KALDI_ASSERT(waveform.NumRows() != 0 && waveform.NumCols() != 0);
	my_cuda_dither(waveform.Data(), waveform.NumRows(), waveform.NumCols(), waveform.Stride(), dither_value, waveform.NumRows(), waveform.NumCols());
}

template<typename Real>
inline void CudaPreemp(CuMatrixBase<Real>& waveform, float coeff){
	KALDI_ASSERT(waveform.NumRows() != 0 && waveform.NumCols() != 0);
	my_cuda_preemphasize(waveform.Data(), waveform.NumRows(), waveform.NumCols(), waveform.Stride(), coeff, waveform.NumRows(), 1);
}

template<typename Real>
inline void CudaWaveSum(const CuMatrixBase<Real>& waveform, CuVectorBase<Real>& wavesum){
	KALDI_ASSERT(waveform.NumRows() != 0 && waveform.NumCols() != 0);
	my_cuda_wave_sum(waveform.Data(), waveform.NumRows(), waveform.NumCols(), waveform.Stride(), wavesum.Data(), waveform.NumRows(), 1);
}

template<typename Real>
inline void CudaWaveMul(CuMatrixBase<Real>& waveform, const CuVectorBase<Real>& window){	
	KALDI_ASSERT(waveform.NumRows() != 0 && waveform.NumCols() != 0);
	KALDI_ASSERT(window.Dim() != 0);
	my_cuda_wave_mul(waveform.Data(), waveform.NumRows(), waveform.NumCols(), waveform.Stride(), window.Data(), waveform.NumRows(), waveform.NumCols());
}

template<typename Real>
inline void CudaSetZero(CuMatrixBase<Real>& waveform){
	KALDI_ASSERT(waveform.NumRows() != 0 && waveform.NumCols() != 0);
	my_cuda_set_zero(waveform.Data(), waveform.NumRows(), waveform.NumCols(), waveform.Stride(), waveform.NumRows(), waveform.Stride() - waveform.NumCols());
}

template<typename Real>
inline void CudaDcOffset(CuMatrixBase<Real>& waveform, const CuVectorBase<Real>& wavesum){
	KALDI_ASSERT(waveform.NumRows() != 0 && waveform.NumCols() != 0);
	my_cuda_wave_dc_offset(waveform.Data(), waveform.NumRows(), waveform.NumCols(), waveform.Stride(), wavesum.Data(), waveform.NumRows(), waveform.NumCols());
}

template<typename Real>
inline void CudaLogEnergy(const CuMatrixBase<Real>& waveform, CuVector<Real>& des){	
	KALDI_ASSERT(waveform.NumRows() != 0 && waveform.NumCols() != 0);
	my_cuda_log_energy(waveform.Data(), waveform.NumRows(), waveform.NumCols(), waveform.Stride(), des.Data());
}

template<typename Real>
inline void CudaSrfft(CuMatrixBase<Real>& waveform){	
	KALDI_ASSERT(waveform.NumRows() != 0 && waveform.NumCols() != 0);
	CuMatrix<Real> temp_buffer(waveform.NumRows(), waveform.NumCols());
	my_cuda_srfft(waveform.Data(), waveform.NumRows(), waveform.Stride(), temp_buffer.Data());
}

template<typename Real>
inline void CudaComputePower(CuMatrixBase<Real> &wave){
	my_cuda_compute_power(wave.Data(), wave.NumRows(), wave.NumCols(), wave.Stride());
}

template<typename Real>
inline void CudaSetEnergy(CuMatrixBase<Real> &des, CuVectorBase<Real> &src, const Real energy_floor, const Real log_energy_floor){
	my_cuda_set_energy(des.Data(), des.NumRows(), des.Stride(), energy_floor, log_energy_floor, src.Data());
}
}
#endif
