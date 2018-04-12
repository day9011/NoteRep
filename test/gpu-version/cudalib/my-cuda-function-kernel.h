#ifndef MY_CUDA_FUNCTION_KERNEL
#define MY_CUDA_FUNCTION_KERNEL

#if HAVE_CUDA == 1

#include "cudalib/my-cuda-function-kernel-ansi.h"
#include "base/kaldi-common.h"
#include "cudamatrix/cu-matrixdim.h"
#include "cudamatrix/cu-common.h"
#include "base/kaldi-error.h"
#include "matrix/matrix-common.h"
#include "cudamatrix/cu-matrix-lib.h"


namespace kaldi{

inline void my_cuda_compute_fft(float *data, int32_cuda dim) { _F_my_cuda_compute_fft(data, dim); }
inline void my_cuda_compute_fft(double *data, int32_cuda dim) { _D_my_cuda_compute_fft(data, dim); }

inline void my_cuda_gmm_select(int32_cuda Gr, int32_cuda Bl, const double *data, MatrixDim d, int32_cuda num_gselect, int32_cuda *gmm_out)
{
	_D_my_cuda_gmm_select(Gr, Bl, data, d, num_gselect, gmm_out);
}

inline void my_cuda_gmm_select(int32_cuda Gr, int32_cuda Bl, const float *data, MatrixDim d, int32_cuda num_gselect, int32_cuda *gmm_out)
{
	_F_my_cuda_gmm_select(Gr, Bl, data, d, num_gselect, gmm_out);
}

inline void my_cuda_add_mat_vec(const float *gamma, const float *Sigma_inv_M_, float *data, int32_cuda gamma_size, int32_cuda Sigma_row, int32_cuda Sigma_col){
    _F_my_cuda_add_mat_vec(gamma, Sigma_inv_M_, data, gamma_size, Sigma_row, Sigma_col);
}

inline void my_cuda_add_mat_vec(const double *gamma, const double *Sigma_inv_M_, double *data, int32_cuda gamma_size, int32_cuda Sigma_row, int32_cuda Sigma_col){
    _D_my_cuda_add_mat_vec(gamma, Sigma_inv_M_, data, gamma_size, Sigma_row, Sigma_col);
}

inline void my_cuda_parallel_memcpy(float *des, float *src, int32_cuda sizes, int32_cuda rows, int32_cuda cols, int32_cuda stride, int32_cuda Gr, int32_cuda Bl){
	_F_my_cuda_parallel_memcpy(des, src, sizes, rows, cols, stride, Gr, Bl);
}

inline void my_cuda_parallel_memcpy(double *des, double *src, int32_cuda sizes, int32_cuda rows, int32_cuda cols, int32_cuda stride, int32_cuda Gr, int32_cuda Bl){
	_D_my_cuda_parallel_memcpy(des, src, sizes, rows, cols, stride, Gr, Bl);
}

inline void my_cuda_vec_memcpy(const float *srcData,int32_cuda srcDim, int32_cuda blank, float *desData, int32_cuda desNumRows, int32_cuda desNumCols, int32_cuda desStride, int32_cuda Gr, int32_cuda Bl){
	_F_my_cuda_vec_memcpy(srcData, srcDim, blank, desData, desNumRows, desNumCols, desStride, Gr, Bl);
}

inline void my_cuda_vec_memcpy(const double *srcData,int32_cuda srcDim, int32_cuda blank, double *desData, int32_cuda desNumRows, int32_cuda desNumCols, int32_cuda desStride, int32_cuda Gr, int32_cuda Bl){
	_D_my_cuda_vec_memcpy(srcData, srcDim, blank, desData, desNumRows, desNumCols, desStride, Gr, Bl);
}

inline void my_cuda_parallel_mel_vecvec(const float *powerData, const int32_cuda powerNumRows, const int32_cuda powerStride, const int32_cuda *offset, const int32_cuda *bins_len, const float *binsData, const int32_cuda binsNumRows, const int32_cuda binsNumCols, float *mel_energies_outData, const int32_cuda mel_energies_outStride, const int32_cuda htk_mode, int32_cuda Gr){
	_F_my_cuda_parallel_mel_vecvec(powerData, powerNumRows, powerStride, offset, bins_len, binsData, binsNumRows, binsNumCols, mel_energies_outData, mel_energies_outStride, htk_mode, Gr);	
}

inline void my_cuda_parallel_mel_vecvec(const double *powerData, const int32_cuda powerNumRows, const int32_cuda powerStride, const int32_cuda *offset, const int32_cuda *bins_len, const double *binsData, const int32_cuda binsNumRows, const int32_cuda binsNumCols, double *mel_energies_outData, const int32_cuda mel_energies_outStride, const int32_cuda htk_mode, int32_cuda Gr){
	_D_my_cuda_parallel_mel_vecvec(powerData, powerNumRows, powerStride, offset, bins_len, binsData, binsNumRows, binsNumCols, mel_energies_outData, mel_energies_outStride, htk_mode, Gr);	
}

// functions for extract window

/*
Adding dither(gauss_val) to each element
*/
inline void my_cuda_dither(float *waveformData, const int32_cuda waveformNumRows, const int32_cuda waveformNumCols, const int32_cuda waveformStride, float dither_value, int32_cuda Gr, int32 Bl){
	_F_my_cuda_dither(waveformData, waveformNumRows, waveformNumCols, waveformStride, dither_value, Gr, Bl);
}

inline void my_cuda_dither(double *waveformData, const int32_cuda waveformNumRows, const int32_cuda waveformNumCols, const int32_cuda waveformStride, double dither_value, int32_cuda Gr, int32 Bl){
	_D_my_cuda_dither(waveformData, waveformNumRows, waveformNumCols, waveformStride, dither_value, Gr, Bl);
}

/*
Preemphasize function
*/
inline void my_cuda_preemphasize(float *waveformData, const int32_cuda waveformNumRows, const int32_cuda waveformNumCols, const int32_cuda waveformStride, const float coeff, int32_cuda Gr, int32_cuda Bl){
	_F_my_cuda_preemphasize(waveformData, waveformNumRows, waveformNumCols, waveformStride, coeff, Gr, Bl);
}

inline void my_cuda_preemphasize(double *waveformData, const int32_cuda waveformNumRows, const int32_cuda waveformNumCols, const int32_cuda waveformStride, const double coeff, int32_cuda Gr, int32_cuda Bl){
	_D_my_cuda_preemphasize(waveformData, waveformNumRows, waveformNumCols, waveformStride, coeff, Gr, Bl);
}

/*
calculating sum for each frame (row) in a wave (matrix)
*/
inline void my_cuda_wave_sum(const float *waveformData, const int32_cuda waveformNumRows, const int32_cuda waveformNumCols, const int32_cuda waveformStride, float *wavesumData, const int32_cuda Gr, const int32_cuda Bl){	
	_F_my_cuda_wave_sum(waveformData, waveformNumRows, waveformNumCols, waveformStride, wavesumData, Gr, Bl);
}

inline void my_cuda_wave_sum(const double *waveformData, const int32_cuda waveformNumRows, const int32_cuda waveformNumCols, const int32_cuda waveformStride, double *wavesumData, const int32_cuda Gr, const int32_cuda Bl){	
	_D_my_cuda_wave_sum(waveformData, waveformNumRows, waveformNumCols, waveformStride, wavesumData, Gr, Bl);
}

/*
multiply mat by a vector element wise
*/
inline void my_cuda_wave_mul(float *waveformData, const int32_cuda waveformNumRows, const int32_cuda waveformNumCols, const int32_cuda waveformStride, const float *windowData, const int32_cuda Gr, const int32_cuda Bl){
	_F_my_cuda_wave_mul(waveformData, waveformNumRows, waveformNumCols, waveformStride, windowData, Gr, Bl);
}

inline void my_cuda_wave_mul(double *waveformData, const int32_cuda waveformNumRows, const int32_cuda waveformNumCols, const int32_cuda waveformStride, const double *windowData, const int32_cuda Gr, const int32_cuda Bl){
	_D_my_cuda_wave_mul(waveformData, waveformNumRows, waveformNumCols, waveformStride, windowData, Gr, Bl);
}

/*
parallel set zero for the elements between col and stride
*/
inline void my_cuda_set_zero(float *waveformData, const int32_cuda waveformNumRows, const int32_cuda waveformNumCols, const int32_cuda waveformStride, const int32_cuda Gr, const int32_cuda Bl){
	_F_my_cuda_set_zero(waveformData, waveformNumRows, waveformNumCols, waveformStride, Gr, Bl);
}

inline void my_cuda_set_zero(double *waveformData, const int32_cuda waveformNumRows, const int32_cuda waveformNumCols, const int32_cuda waveformStride, const int32_cuda Gr, const int32_cuda Bl){
	_D_my_cuda_set_zero(waveformData, waveformNumRows, waveformNumCols, waveformStride, Gr, Bl);
}

/*
parallel processing dc offset
*/
inline void my_cuda_wave_dc_offset(float *waveformData, const int32_cuda waveformNumRows, const int32_cuda waveformNumCols, const int32_cuda waveformStride, const float *wavesumData, const int32_cuda Gr, const int32_cuda Bl){
	_F_my_cuda_wave_dc_offset(waveformData, waveformNumRows, waveformNumCols, waveformStride, wavesumData, Gr, Bl);
}

inline void my_cuda_wave_dc_offset(double *waveformData, const int32_cuda waveformNumRows, const int32_cuda waveformNumCols, const int32_cuda waveformStride, const double *wavesumData, const int32_cuda Gr, const int32_cuda Bl){
	_D_my_cuda_wave_dc_offset(waveformData, waveformNumRows, waveformNumCols, waveformStride, wavesumData, Gr, Bl);
}

inline void my_cuda_log_energy(const float *waveformData, const int32_cuda waveformNumRows, const int32_cuda waveformNumCols, const int32_cuda waveformStride, float *desData){
	_F_my_cuda_log_energy(waveformData, waveformNumRows, waveformNumCols, waveformStride, desData);
}

inline void my_cuda_log_energy(const double *waveformData, const int32_cuda waveformNumRows, const int32_cuda waveformNumCols, const int32_cuda waveformStride, double *desData){
	_D_my_cuda_log_energy(waveformData, waveformNumRows, waveformNumCols, waveformStride, desData);
}

inline void my_cuda_srfft(float *wave, int32_cuda rows, int32_cuda stride, float *temp_buffer){
	_F_my_cuda_srfft(wave, rows, stride, temp_buffer);
}

inline void my_cuda_srfft(double *wave, int32_cuda rows, int32_cuda stride, double *temp_buffer){
	_D_my_cuda_srfft(wave, rows, stride, temp_buffer);
}

inline void my_cuda_compute_power(float *waveData, const int32_cuda waveNumRows, const int32_cuda waveNumCols, const int32_cuda waveStride){
	_F_my_cuda_compute_power(waveData, waveNumRows, waveNumCols, waveStride);
}

inline void my_cuda_compute_power(double *waveData, const int32_cuda waveNumRows, const int32_cuda waveNumCols, const int32_cuda waveStride){
	_D_my_cuda_compute_power(waveData, waveNumRows, waveNumCols, waveStride);
}

inline void my_cuda_set_energy(float *desData, const int32_cuda desNumRows, const int32_cuda desStride, const float energy_floor, const float log_energy_floor, const float *srcData){
	_F_my_cuda_set_energy(desData, desNumRows, desStride, energy_floor, log_energy_floor, srcData);
}

inline void my_cuda_set_energy(double *desData, const int32_cuda desNumRows, const int32_cuda desStride, const float energy_floor, const double log_energy_floor, const double *srcData){
	_D_my_cuda_set_energy(desData, desNumRows, desStride, energy_floor, log_energy_floor, srcData);
}

inline void my_cuda_scale_linear(float *A, int32_cuda dim, float alpha, int32_cuda Gr, int32_cuda Bl)
{
	_F_my_cuda_scale_linear(A, dim, alpha, Gr, Bl);
}

inline void my_cuda_scale_linear(double *A, int32_cuda dim, double alpha, int32_cuda Gr, int32_cuda Bl)
{
	_D_my_cuda_scale_linear(A, dim, alpha, Gr, Bl);
}

inline void my_cuda_addvec2(float *A, const float *x, int32_cuda dim, float alpha, dim3 Gr, dim3 Bl)
{
	_F_my_cuda_addvec2(A, x, dim, alpha, Gr, Bl);
}

inline void my_cuda_addvec2(double *A, const double *x, int32_cuda dim, double alpha, dim3 Gr, dim3 Bl)
{
	_D_my_cuda_addvec2(A, x, dim, alpha, Gr, Bl);
}

inline void my_cuda_addvec3(float *A, int32_cuda numA, const float *x, MatrixDim d,float alpha, dim3 Gr, dim3 Bl)
{
	_F_my_cuda_addvec3(A, numA, x, d, alpha, Gr, Bl);
}

inline void my_cuda_addvec3(double *A, int32_cuda numA, const double *x, MatrixDim d, double alpha, dim3 Gr, dim3 Bl)
{
	_D_my_cuda_addvec3(A, numA, x, d, alpha, Gr, Bl);
}

inline void my_cuda_MatVecVec(const float *A, const float *B, MatrixDim dA, MatrixDim dB, float *x, int32_cuda Gr, int32_cuda Bl)
{
	_F_my_cuda_MatVecVec(A, B, dA, dB, x, Gr, Bl);
}

inline void my_cuda_MatVecVec(const double *A, const double *B, MatrixDim dA, MatrixDim dB, double *x, int32_cuda Gr, int32_cuda Bl)
{
	_D_my_cuda_MatVecVec(A, B, dA, dB, x, Gr, Bl);
}

inline void my_cuda_LogLikelihoodsPreselect(const int32_cuda *gselect, int32_cuda gselect_rows, int32_cuda gselect_cols, const float *features, MatrixDim d_features, const float *gconsts_, int32_cuda dim_gconsts, const float *means_invcovars_, MatrixDim d_means, const float *data_sqs, const float *inv_covars_, int32_cuda spdim, float *loglikes, MatrixDim d_loglikes, dim3 Gr, dim3 Bl)
{
	_F_my_cuda_LogLikelihoodsPreselect(gselect, gselect_rows, gselect_cols, features, d_features, gconsts_, dim_gconsts, means_invcovars_, d_means, data_sqs, inv_covars_, spdim, loglikes, d_loglikes, Gr, Bl);
}

inline void my_cuda_LogLikelihoodsPreselect(const int32_cuda *gselect, int32_cuda gselect_rows, int32_cuda gselect_cols, const double *features, MatrixDim d_features, const double *gconsts_, int32_cuda dim_gconsts, const double *means_invcovars_, MatrixDim d_means, const double *data_sqs, const double *inv_covars_, int32_cuda spdim, double *loglikes, MatrixDim d_loglikes, dim3 Gr, dim3 Bl)
{
	_D_my_cuda_LogLikelihoodsPreselect(gselect, gselect_rows, gselect_cols, features, d_features, gconsts_, dim_gconsts, means_invcovars_, d_means, data_sqs, inv_covars_, spdim, loglikes, d_loglikes, Gr, Bl);
}

inline void my_cuda_scale_diag_numsp(float *A, int32_cuda nums, MatrixDim d, float alpha, dim3 Gr, dim3 Bl)
{
	_F_my_cuda_scale_diag_numsp(A, nums, d, alpha, Gr, Bl);
}

inline void my_cuda_scale_diag_numsp(double *A, int32_cuda nums, MatrixDim d, double alpha, dim3 Gr, dim3 Bl)
{
	_D_my_cuda_scale_diag_numsp(A, nums, d, alpha, Gr, Bl);
}

inline void my_cuda_compute_posterior(float *loglikes, MatrixDim d_log, float min_post, int32_cuda Gr, int32_cuda Bl)
{
	_F_my_cuda_compute_posterior(loglikes, d_log, min_post, Gr, Bl);
}

inline void my_cuda_compute_posterior(double *loglikes, MatrixDim d_log, double min_post, int32_cuda Gr, int32_cuda Bl)
{
	_D_my_cuda_compute_posterior(loglikes, d_log, min_post, Gr, Bl);
}

inline void my_cuda_MatApplySoftMax(float *data, MatrixDim d, int32_cuda Gr, int32_cuda Bl)
{
	_F_my_cuda_MatApplySoftMax(data, d, Gr, Bl);
}

inline void my_cuda_MatApplySoftMax(double *data, MatrixDim d, int32_cuda Gr, int32_cuda Bl)
{
	_D_my_cuda_MatApplySoftMax(data, d, Gr, Bl);
}

inline void my_cuda_AddMatColsToVec(const float *mat, MatrixDim d, float *vec, int32_cuda Gr, int32_cuda Bl)
{
	_F_my_cuda_AddMatColsToVec(mat, d, vec, Gr, Bl);
}

inline void my_cuda_AddMatColsToVec(const double *mat, MatrixDim d, double *vec, int32_cuda Gr, int32_cuda Bl)
{
	_D_my_cuda_AddMatColsToVec(mat, d, vec, Gr, Bl);
}
}
#endif

#endif
