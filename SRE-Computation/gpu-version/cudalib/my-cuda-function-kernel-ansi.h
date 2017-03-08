#ifndef MY_CUDA_FUNTION_KERNEL_ANSI
#define MY_CUDA_FUNTION_KERNEL_ANSI
#include "cudamatrix/cu-matrixdim.h"
#include <cuda.h>
#include <cuda_runtime.h>

#if HAVE_CUDA == 1

extern "C"
{
	void _F_my_cuda_compute_fft(float *data, int dim);
	
	void _D_my_cuda_compute_fft(double *data, int dim);
	
	void _F_my_cuda_gmm_select(int32_cuda Gr, int32_cuda Bl, const float *data, MatrixDim d, int32_cuda num_gselect, int32_cuda *gmm_out);
	void _D_my_cuda_gmm_select(int32_cuda Gr, int32_cuda Bl, const double *data, MatrixDim d, int32_cuda num_gselect, int32_cuda *gmm_out);
	void _F_my_cuda_add_mat_vec(const float *gamma, const float *Sigma_inv_M_, float *data, int32_cuda gamma_size, int32_cuda Sigma_row, int32_cuda Sigma_col);
	void _D_my_cuda_add_mat_vec(const double *gamma,const double *Sigma_inv_M_, double *data, int32_cuda gamma_size, int32_cuda Sigma_row, int32_cuda Sigma_col);
	void _F_my_cuda_parallel_memcpy(float *des, float *src, int32_cuda sizes, int32_cuda rows, int32_cuda cols, int32_cuda stride, int32_cuda Gr, int32_cuda Bl);
	void _D_my_cuda_parallel_memcpy(double *des, double *src, int32_cuda sizes, int32_cuda rows, int32_cuda cols, int32_cuda stride, int32_cuda Gr, int32_cuda Bl);
	void _F_my_cuda_vec_memcpy(const float *src, int32_cuda len, int32_cuda blank, float *des, int32_cuda rows, int32_cuda cols, int32_cuda stride, int32_cuda Gr, int32_cuda Bl);
	void _D_my_cuda_vec_memcpy(const double *src, int32_cuda len, int32_cuda blank, double *des, int32_cuda rows, int32_cuda cols, int32_cuda stride, int32_cuda Gr, int32_cuda Bl);
	void _F_my_cuda_parallel_mel_vecvec(const float *voice, const int32_cuda rows, const int32_cuda cols, const int32_cuda *offset, const int32_cuda *bins_len, const float *bins, const int32_cuda bin_rows, const int32_cuda bin_cols, float *mel_energies_out, const int32_cuda mel_stride, int32_cuda htk_mode, int32_cuda Gr);
	void _D_my_cuda_parallel_mel_vecvec(const double *voice, const int32_cuda rows, const int32_cuda cols, const int32_cuda *offset, const int32_cuda *bins_len, const double *bins, const int32_cuda bin_rows, const int32_cuda bin_cols, double *mel_energies_out, const int32_cuda mel_stride, int32_cuda htk_mode, int32_cuda Gr);
	void _F_my_cuda_process_gauss_val(float *val, int32_cuda len, const int32_cuda Gr, const int32_cuda Bl);
	void _D_my_cuda_process_gauss_val(double *val, int32_cuda len, const int32_cuda Gr, const int32_cuda Bl);
	void _F_my_cuda_dither(float *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda matrix_stride, float dither_value, const int32_cuda Gr, const int32_cuda Bl);
	void _D_my_cuda_dither(double *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda matrix_stride, double dither_value, const int32_cuda Gr, const int32_cuda Bl);
	void _F_my_cuda_preemphasize(float *waveform, int32_cuda rows, int32_cuda cols, int32_cuda matrix_stride, float coeff, const int32_cuda Gr, const int32_cuda Bl);
	void _D_my_cuda_preemphasize(double *waveform, int32_cuda rows, int32_cuda cols, int32_cuda matrix_stride, double coeff, const int32_cuda Gr, const int32_cuda Bl);
	void _F_my_cuda_wave_sum(const float *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda matrix_stride, float *wave_sum, const int32_cuda Gr, const int32_cuda Bl);
	void _D_my_cuda_wave_sum(const double *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda matrix_stride, double *wave_sum, const int32_cuda Gr, const int32_cuda Bl);
	void _F_my_cuda_wave_mul(float *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda stride, const float *window, const int32_cuda Gr, const int32_cuda Bl);
	void _D_my_cuda_wave_mul(double *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda stride, const double *window, const int32_cuda Gr, const int32_cuda Bl);
	void _F_my_cuda_set_zero(float *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda matrix_stride, const int32_cuda Gr, const int32_cuda Bl);
	void _D_my_cuda_set_zero(double *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda matrix_stride, const int32_cuda Gr, const int32_cuda Bl);
	void _F_my_cuda_wave_dc_offset(float *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda matrix_stride, const float *wave_sum, const int32_cuda Gr, const int32_cuda Bl);
	void _D_my_cuda_wave_dc_offset(double *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda matrix_stride, const double *wave_sum, const int32_cuda Gr, const int32_cuda Bl);
	void _F_my_cuda_log_energy(const float *src, int32_cuda rows, int32_cuda cols, int32_cuda stride, float *des);
	void _D_my_cuda_log_energy(const double *src, int32_cuda rows, int32_cuda cols, int32_cuda stride, double *des);
	void _F_my_cuda_srfft(float *wave, int32_cuda rows, int32_cuda stride, float *temp_buffer);
	void _D_my_cuda_srfft(double *wave, int32_cuda rows, int32_cuda stride, double *temp_buffer);
	void _F_my_cuda_compute_power(float *waveform, int32_cuda rows, int32_cuda cols, int32_cuda stride);
	void _D_my_cuda_compute_power(double *waveform, int32_cuda rows, int32_cuda cols, int32_cuda stride);
	void _F_my_cuda_set_energy(float *des, const int32_cuda rows, const int32_cuda stride, const float energy_floor, const float log_energy_floor, const float *src);
	void _D_my_cuda_set_energy(double *des, const int32_cuda rows, const int32_cuda stride, const float energy_floor, const double log_energy_floor, const double *src);
	void _F_my_cuda_scale_linear(float *A, int32_cuda dim, float alpha, int32_cuda Gr, int32_cuda Bl);
	void _D_my_cuda_scale_linear(double *A, int32_cuda dim, double alpha, int32_cuda Gr, int32_cuda Bl);
	void _F_my_cuda_addvec2(float *A, const float *x, int32_cuda dim, float alpha, dim3 Gr, dim3 Bl);
	void _D_my_cuda_addvec2(double *A, const double *x, int32_cuda dim, double alpha, dim3 Gr, dim3 Bl);
	void _F_my_cuda_addvec3(float *A, int32_cuda numA, const float *x, MatrixDim d,  float alpha, dim3 Gr, dim3 Bl);
	void _D_my_cuda_addvec3(double *A, int32_cuda numA, const double *x, MatrixDim d,  double alpha, dim3 Gr, dim3 Bl);
	void _F_my_cuda_MatVecVec(const float *A, const float *B, MatrixDim dA, MatrixDim dB, float *x, int32_cuda Gr, int32_cuda Bl);
	void _D_my_cuda_MatVecVec(const double *A, const double *B, MatrixDim dA, MatrixDim dB, double *x, int32_cuda Gr, int32_cuda Bl);
	void _F_my_cuda_LogLikelihoodsPreselect(const int32_cuda *gselect, int32_cuda gselect_rows, int32_cuda gselect_cols, const float *features, MatrixDim d_features, const float *gconsts_, int32_cuda dim_gconsts, const float *means_invcovars_, MatrixDim d_means, const float *data_sqs, const float *inv_covars_, int32_cuda spdim, float *loglikes, MatrixDim d_loglikes, dim3 Gr, dim3 Bl);
	void _D_my_cuda_LogLikelihoodsPreselect(const int32_cuda *gselect, int32_cuda gselect_rows, int32_cuda gselect_cols, const double *features, MatrixDim d_features, const double *gconsts_, int32_cuda dim_gconsts, const double *means_invcovars_, MatrixDim d_means, const double *data_sqs, const double *inv_covars_, int32_cuda spdim, double *loglikes, MatrixDim d_loglikes, dim3 Gr, dim3 Bl);
	void _F_my_cuda_scale_diag_numsp(float *A, int32_cuda nums, MatrixDim d, float alpha, dim3 Gr, dim3 Bl);
	void _D_my_cuda_scale_diag_numsp(double *A, int32_cuda nums, MatrixDim d, double alpha, dim3 Gr, dim3 Bl);
	void _F_my_cuda_compute_posterior(float *loglikes, MatrixDim d_log, float min_post, int32_cuda Gr, int32_cuda Bl);
	void _D_my_cuda_compute_posterior(double *loglikes, MatrixDim d_log, double min_post, int32_cuda Gr, int32_cuda Bl);
	void _F_my_cuda_MatApplySoftMax(float *data, MatrixDim d, int32_cuda Gr, int32_cuda Bl);
	void _D_my_cuda_MatApplySoftMax(double *data, MatrixDim d, int32_cuda Gr, int32_cuda Bl);
	void _F_my_cuda_AddMatColsToVec(const float *mat, MatrixDim d, float *vec, int32_cuda Gr, int32_cuda Bl);
	void _D_my_cuda_AddMatColsToVec(const double *mat, MatrixDim d, double *vec, int32_cuda Gr, int32_cuda Bl);

}

#endif //HAVE_CUDA

#endif
