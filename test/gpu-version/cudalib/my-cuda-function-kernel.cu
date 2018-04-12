#include <curand.h>
#include <cfloat>
#include <cufft.h>
#include <cuda.h>
#include <stdio.h>
#include <stdlib.h>
#include "my-cuda-function-kernel-ansi.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <limits>
#include <cublas_v2.h>

#define PI 3.1415926535897932384626433832795

#define CUDA_CALL(ret) \
{\
  if((ret) != cudaSuccess) { \
  printf("Error at %s:%d\n", __FILE__, __LINE__); \
  printf("Error code %s\n", cudaGetErrorString(ret)); \
  exit(-1); \
  } \
  cudaThreadSynchronize(); \
}

#define CURAND_CALL(x)\
{\
	if((x) != CURAND_STATUS_SUCCESS) {\
		printf("Error at %s:%d\n",__FILE__,__LINE__); \
		exit(-1); \
	}\
} 
using namespace std;

//compute matrix sum of one column
template<typename Real>
__device__
static Real _sum_col(const Real *x, MatrixDim d, int32_cuda col)
{
	Real sum = 0;
	for(int32_cuda i = 0; i < d.rows; i++)
		sum += x[i * d.stride + col];
	return sum;
}

//compute dot product between two vectors.
template<typename Real>
__device__
static void _VecVec(const Real *x, const Real *y, int32_cuda dim, Real *res)
{
	Real result = 0;
	for (int32_cuda i = 0; i < dim; i++)
		result += x[i] * y[i];
	*res = result;
}

//Scale all element
template<typename Real>
__device__
static void __Scale(Real *x, int32_cuda dim, Real alpha)
{
	for (int32_cuda i = 0; i < dim; i++)
		x[i] = x[i] * alpha;
}

//select max number which is not zero
template<typename Real>
__device__
static Real __Max(const Real *data, int32_cuda dim)
{
	Real ans = (Real)(-1000000);
	for (int32_cuda i = 0; i < dim; i++)
		if (data[i] > ans && data[i] != 0) ans = data[i];
	return ans;
}

//select max number index which value is not zero
template<typename Real>
__device__
static int32_cuda __Max_index(const Real *data, int32_cuda dim)
{
	Real ans = (Real)(-1000000);
	int32_cuda index = 0;
	for (int32_cuda i = 0; i < dim; i++)
		if (data[i] > ans && data[i] != 0) { ans = data[i]; index = i; }
	return index;
}


template<typename Real>
__device__
static Real __Sum(Real *data, int32_cuda dim)
{
	Real sum = 0;
	for (int32_cuda i = 0; i < dim; i++)
		sum += data[i];
	return sum;
}

template<typename Real>
__device__
static void __insert_sort(Real *__first, Real *__last)
{
	if (__first == __last)
		return;
	Real *p;
	for (Real *iter = __first + 1; iter != __last; ++iter)
	{
		Real tmp = *iter;
		for (p = iter; p != __first && tmp < *(p - 1); --p)
			*p = *(p - 1);
		*p = tmp;
	}
}

template<typename Real>
__device__
static Real* __partition(Real *__first, Real *__last, Real __pivot)
{
	while(true)
	{
		while (*__first < __pivot)
			++__first;
		--__last;
		while (__pivot < *__last)
			--__last;
		if(!(__first < __last))
			return __first;
		//swap two number, use plus function to swap two number will lost precision.
		{
			Real temp = *__first;
			*__first = *__last;
			*__last = temp;
		}
		++__first;
	}
}

template<typename Real>
__device__
static void _partition(Real *__first, Real *__nth, Real *__last)
{
	while(__last - __first > 3)
	{
		Real *__cut = __partition(__first, __last, *(__first + (__last - __first) / 2));
		if (__cut <= __nth)
			__first = __cut;
		else
			__last = __cut;
	}
	__insert_sort(__first, __last);
}

template<typename Real>
__global__
static void _gmm_select(const Real *data, MatrixDim d, Real *copydata, MatrixDim c_d, int32_cuda num_gselect, int32_cuda *gmm_selected)
{
	int32_cuda row = blockDim.x * blockIdx.x + threadIdx.x;
	if (row < d.rows)
	{
		_partition(copydata + row * c_d.stride, copydata + row * c_d.stride + c_d.cols - num_gselect, copydata + row * c_d.stride + c_d.cols);
		Real thresh = copydata[row * c_d.stride + c_d.cols - num_gselect];
		int32_cuda index = 0;
		for (int32_cuda j = 0; j < d.cols; j++)
			if (*(data + row * d.stride + j) >= thresh)
			{
				if(index < 20)
				{
					*(gmm_selected + row * num_gselect + index) = j;
					++index;
				}
				else break;
			}
		__syncthreads();
	}
}

template<typename Real>
__global__
static void _MatApplySoftMax(Real *data, MatrixDim d)
{
	int32_cuda i = blockIdx.x * blockDim.x + threadIdx.x;
	if (i < d.rows)
	{
		Real sum = 0;
		Real max = __Max(data + i * d.stride, d.cols);
		for (int32_cuda j = 0; j < d.cols; j++)
		{
			if (data[i * d.stride + j] != 0)
			{
				data[i * d.stride + j] = exp(data[i * d.stride + j] - max);
				sum += data[i * d.stride + j];
			}
		}
		__Scale(data + i * d.stride, d.cols, (Real)(1.0 / sum));
		__syncthreads();
	}
}

template<typename Real>
__global__
static void _compute_posterior(Real *loglikes, MatrixDim d_log, Real min_post)
{
	int32_cuda i = blockIdx.x * blockDim.x + threadIdx.x;
	if (i < d_log.rows)
	{
		if (min_post != 0.0)
		{
			int32_cuda max_index = __Max_index(loglikes + i * d_log.stride, d_log.cols);
			for (int32_cuda j = 0; j < d_log.cols; j++)
				if (loglikes[i * d_log.stride + j] < min_post)
					loglikes[i * d_log.stride + j] = 0.0;
			Real sum = __Sum(loglikes + i * d_log.stride, d_log.cols);
			if (sum == 0.0)
				loglikes[i * d_log.stride + max_index] = 1.0;
			else
				__Scale(loglikes + i * d_log.stride, d_log.cols, (Real)(1.0 / sum));
		}
		__syncthreads();
	}
}

//get a vector by sum of elements of every cols
template<typename Real>
__global__
static void _add_cols_mat_to_vec(const Real *mat, MatrixDim d, Real *vec)
{
	int32_cuda i = blockIdx.x * blockDim.x + threadIdx.x;
	if (i < d.cols)
	{
		vec[i] = _sum_col(mat, d, i);
	}
}

template<typename Real>
__global__
static void _scale_linear(Real *A, int32_cuda dim, Real alpha)
{
	int32_cuda i = blockIdx.x * blockDim.x + threadIdx.x;
	if (i < dim)
	{
		A[i] = A[i] * alpha;
	}
}

template<typename Real>
__global__
static void _scale_diag_numsp(Real *A, int32_cuda nums, MatrixDim d, Real alpha)
{
	int32_cuda i = blockIdx.x * blockDim.x + threadIdx.x;
	int32_cuda j = blockIdx.y * blockDim.y + threadIdx.y;
	if (i < nums)
	{
		if (j < d.cols)
		{
			int32_cuda index = (j + 1) * (j + 2) / 2 - 1;
 			A[i * d.stride + index] = A[i * d.stride + index] * alpha;
			__syncthreads();
		}
	}
}

template <typename Real>
__global__
static void _my_addvec2(Real *A, const Real *x, int32_cuda dim, Real alpha)
{
	int32_cuda i = blockIdx.x * blockDim.x + threadIdx.x;
	int32_cuda j = blockIdx.y * blockDim.y + threadIdx.y;
	if (i <= j && j < dim)
	{
		A[j * (j + 1) / 2 + i] = x[j] * x[i] * alpha;
		__syncthreads();
	}
}

template<typename Real>
__global__
static void _my_addvec3(Real *A, int32_cuda numA, const Real *x, MatrixDim d, Real alpha)
{
    int32_cuda i = blockIdx.x * blockDim.x + threadIdx.x;
    int32_cuda j = blockIdx.y * blockDim.y + threadIdx.y;
    int32_cuda k = blockIdx.z * blockDim.z + threadIdx.z;
	/* int maxi = 0, maxj = 0, maxk = 0; */
	/* if (i > maxi) maxi = i; */
	/* if (j > maxj) maxj = j; */
	/* if (k > maxk) maxk = k; */
    int32_cuda stride = d.cols * (d.cols + 1) / 2;
    if (i < numA && j < d.cols && k < d.cols)
    {
	    if(k <= j)
	    {
			A[i * stride + j * (j + 1) / 2 + k] = x[i * d.stride + j] * x[i * d.stride + k] * alpha;
		}
	    __syncthreads();
		/* if (i > (numA - 3) && j > (d.cols - 3) && k > (d.cols - 3)) */
		/*     printf("\nmax i: %d, max j: %d, max k: %d\n", maxi, maxj, maxk); */
	}
}

template<typename Real>
__global__
static void printdata(Real *data, MatrixDim d)
{
	for (int r = 0; r < 2; r++)
	{
		printf("[");
		for (int c = 0; c < d.cols; c++)
			printf(" %g", data[1000 * d.stride + c]);
		printf(" ]");
	}
}

template<typename Real>
__host__
static void _my_cuda_AddMatColsToVec(const Real *mat, MatrixDim d, Real *vec, int32_cuda Gr, int32_cuda Bl)
{
	_add_cols_mat_to_vec<<<Gr, Bl>>>(mat, d, vec);
}

template<typename Real>
__host__
static void _my_cuda_MatApplySoftMax(Real *data, MatrixDim d, int32_cuda Gr, int32_cuda Bl)
{
	_MatApplySoftMax<<<Gr, Bl>>>(data, d);
}

template<typename Real>
__host__
static void _my_cuda_scale_diag_numsp(Real *A, int32_cuda nums, MatrixDim d, Real alpha, dim3 Gr, dim3 Bl)
{
	_scale_diag_numsp<<<Gr,Bl>>>(A, nums, d, alpha);
}

template<typename Real>
__host__
static void _my_cuda_scale_linear(int32_cuda Gr, int32_cuda Bl, Real *A, int32_cuda dim, Real alpha)
{
	_scale_linear<<<Gr, Bl>>>(A, dim, alpha);
}

template<typename Real>
__host__
static void _my_cuda_gmm_select(int32_cuda Gr, int32_cuda Bl, const Real *data, MatrixDim d, int32_cuda num_gselect, int32_cuda *gmm_out)
{
	Real *copydata;
	size_t pitch;
	MatrixDim c_d;
	c_d.rows = d.rows;
	c_d.cols = d.cols;
	CUDA_CALL(cudaMallocPitch((void **)&copydata, &pitch, d.cols * sizeof(Real), d.rows));
	c_d.stride = pitch / sizeof(Real);
	CUDA_CALL(cudaMemcpy2D(copydata, c_d.stride * sizeof(Real), data, d.stride * sizeof(Real), d.cols * sizeof(Real), d.rows, cudaMemcpyDeviceToDevice));
	cudaDeviceSynchronize();
	_gmm_select<<<Gr, Bl>>>(data, d, copydata, c_d, num_gselect, gmm_out);
	CUDA_CALL(cudaFree(copydata));
}


template<typename Real>
__host__
static void _my_cuda_compute_fft(Real *data, int32_cuda dim)
{
	cufftComplex *CompData = (cufftComplex *)malloc(dim * sizeof(cufftComplex));
	for (int32_cuda i = 0; i < dim; i++)
	{
		CompData[i].x = data[i];
		CompData[i].y = 0;
	}
	cufftComplex *devData;
	CUDA_CALL(cudaMalloc((void **)&devData, dim * sizeof(cufftComplex)));
	CUDA_CALL(cudaMemcpy(devData, CompData, dim * sizeof(CompData), cudaMemcpyHostToDevice));

	cufftHandle plan;
	cufftPlan1d(&plan, dim, CUFFT_C2C, 1);
	cufftExecC2C(plan, devData, devData, CUFFT_FORWARD);
	cudaDeviceSynchronize();
	CUDA_CALL(cudaMemcpy(CompData, devData, dim * sizeof(cufftComplex), cudaMemcpyDeviceToHost));
	for (int32_cuda i = 0; i < dim / 2; i++)
	{
		data[2 * i] = CompData[i].x;
		data[2 * i + 1] = CompData[i].y;
	}
	data[1] = CompData[dim / 2].x;
	CUDA_CALL(cudaFree(devData));
	free(CompData);
}

// 基于cuBlas实现的矩阵和向量的乘法
template<typename Real>
__host__
void _add_mat_vec(const Real *gamma, const Real *Sigma_inv_M_, Real *data, int32_cuda gamma_size, int32_cuda Sigma_row, int32_cuda Sigma_col){
	Real alpha = 1.0f;
	Real beta = 0.0f;
	cublasHandle_t handle;  
	
	cublasCreate(&handle); 	
	mySgemv(handle, CUBLAS_OP_N, Sigma_col, Sigma_row, &alpha, Sigma_inv_M_, Sigma_col, gamma, 1, &beta, data, 1);
	cublasDestroy(handle);	
}

cublasStatus_t mySgemv(cublasHandle_t &handle, cublasOperation_t trans, int m, int n, const float *alpha, const float *A, int lda, const float *x, int incx, const float *beta, float *y, int incy){
	return cublasSgemv_v2(handle, trans, m, n, alpha, A, lda, x, incx, beta, y, incy);
}

cublasStatus_t mySgemv(cublasHandle_t &handle, cublasOperation_t trans, int m, int n, const double *alpha, const double *A, int lda, const double *x, int incx, const double *beta, double *y, int incy){
	return cublasDgemv_v2(handle, trans, m, n, alpha, A, lda, x, incx, beta, y, incy);

}

//Parallel memory copy to cuda variables
template<typename Real>
__global__
static void _parallel_memcpy(Real *des, Real *src, int32_cuda sizes, int32_cuda rows, int32_cuda cols, int32_cuda stride){
	if(threadIdx.x <= cols && threadIdx.x <= stride){
		int32_cuda des_id = threadIdx.x + blockIdx.x * cols;
		int32_cuda src_id = threadIdx.x + blockIdx.x * stride;
		int32_cuda gen_stride = gridDim.x;
    	int32_cuda stridex = 0;

		for(;src_id + stridex * stride < sizes * rows * stride; stridex += gen_stride){	
			*(des + des_id + stridex * cols) = *(src + src_id + stridex * stride);
		}
	}
}

// compute vecvec between two matrixs
template<typename Real>
__global__
static void _MatVecVec(const Real *A, const Real *B, int32_cuda rows, int32_cuda cols, int32_cuda strideA, int32_cuda strideB, Real *out)
{
	int32_cuda i = blockIdx.x * blockDim.x + threadIdx.x;
	if (i < rows)
	{
		_VecVec(A + i * strideA, B + i * strideB, cols, out + i);
	}
}

template<typename Real>
__global__
static void _LogLikelihoodsPreselect(const int32_cuda *gselect, int32_cuda gselect_rows, int32_cuda gselect_cols, const Real *features, MatrixDim d_features, const Real *gconsts_, int32_cuda dim_gconsts, const Real *means_invcovars_, MatrixDim d_means, const Real *data_sqs, const Real *inv_covars_, int32_cuda spdim, Real *loglikes, MatrixDim d_loglikes)
{
	int32_cuda i = blockIdx.x * blockDim.x + threadIdx.x;
	int32_cuda j = blockIdx.y * blockDim.y + threadIdx.y;
	if (i < d_features.rows && j < gselect_cols)
	{
		int32_cuda idx = gselect[i * gselect_cols + j];
		Real means_inv_data = 0, data_sq_inv = 0;
		_VecVec(means_invcovars_ + idx * d_means.stride, features + i * d_features.stride, d_features.cols, &means_inv_data);
		_VecVec(data_sqs + i * spdim, inv_covars_ + idx * spdim, spdim, &data_sq_inv);
		loglikes[i * d_loglikes.stride + idx] = gconsts_[idx] + means_inv_data - data_sq_inv;
		__syncthreads();
	}
}

template<typename Real>
__host__
static void _my_cuda_LogLikelihoodsPreselect(const int32_cuda *gselect, int32_cuda gselect_rows, int32_cuda gselect_cols, const Real *features, MatrixDim d_features, const Real *gconsts_, int32_cuda dim_gconsts, const Real *means_invcovars_, MatrixDim d_means, const Real *data_sqs, const Real *inv_covars_, int32_cuda spdim, Real *loglikes, MatrixDim d_loglikes, dim3 Gr, dim3 Bl)
{
	_LogLikelihoodsPreselect<<<Gr, Bl>>>(gselect, gselect_rows, gselect_cols, features, d_features, gconsts_, dim_gconsts, means_invcovars_, d_means, data_sqs, inv_covars_, spdim, loglikes, d_loglikes);
}


template<typename Real>
__host__
static void _my_cuda_MatVecVec(const Real *A, const Real *B, MatrixDim dA, MatrixDim dB, Real *out, int32_cuda Gr, int32_cuda Bl)
{
	_MatVecVec<<<Gr, Bl>>>(A, B, dA.rows, dA.cols, dA.stride, dB.stride, out);
}

template<typename Real>
__host__
static void _my_parallel_memcpy(Real *des, Real *src, int32_cuda sizes, int32_cuda rows, int32_cuda cols, int32_cuda stride, int32_cuda Gr, int32_cuda Bl){
	_parallel_memcpy<<<Gr ,Bl>>>(des, src, sizes, rows, cols, stride);
}

//Parallel memory copy from vec to mat 
//by default: (rows - 1) * cols <= element(src) <= rows * cols
template<typename Real>
__global__ 
static void _vec_memcpy(const Real *src, int32_cuda len, int32_cuda blank, Real *des, int32_cuda rows, int32_cuda cols, int32_cuda stride){
	if(threadIdx.x >= cols || blockIdx.x >= rows) return;
	int32_cuda row = blockIdx.x;
	int32_cuda row_stride = gridDim.x;
	
	while(row < rows){
		int32_cuda col = threadIdx.x;
		int32_cuda col_stride = blockDim.x;
		int32_cuda cur_idx = row * blank;
		while(col < cols && cur_idx + col + row*cols < len){
			*(des + col + row * stride) = *(src + cur_idx + col + row * cols);
			col += col_stride;
		}
		row += row_stride;
	}
}

template<typename Real>
__host__
static void _cuda_vec_memcpy(const Real *src, int32_cuda len, int32_cuda blank, Real *des, int32_cuda rows, int32_cuda cols, int32_cuda stride, int32_cuda Gr, int32_cuda Bl){
	_vec_memcpy<<<Gr ,Bl>>>(src, len, blank, des, rows, cols, stride);
}

// computing mel energies
// For efficiency: grid = frames, block = bins_len
template<typename Real>
__global__
static void _parallel_mel_vecvec(const Real *voice, const int32_cuda rows, const int32_cuda cols, const int32_cuda *offset, const int32_cuda *bins_len, const Real *bins, const int32_cuda bin_rows, const int32_cuda bin_cols, Real *mel_energies_out, const int32_cuda mel_stride, int32_cuda htk_mode){
	int32_cuda tid = threadIdx.x;
	int32_cuda frame_num = blockIdx.x;
	Real temp = 0.0;
	
	for(int32_cuda dimx = 0; dimx < *(bins_len + tid); dimx += 1){			
		temp += *(voice + frame_num * cols + (*(offset + tid)) + dimx) * (*(bins + dimx + tid * bin_cols));
	}
	*(mel_energies_out + tid + frame_num * mel_stride) = (htk_mode == 0 && temp < 1)? 1 : temp;
}

template<typename Real>
__host__
static void _my_cuda_parallel_mel_vecvec(const Real *voice, const int32_cuda rows, const int32_cuda cols, const int32_cuda *offset, const int32_cuda *bins_len, const Real *bins, const int32_cuda bin_rows, const int32_cuda bin_cols, Real *mel_energies_out, int32_cuda mel_stride, int32_cuda htk_mode, int32_cuda Gr){
	_parallel_mel_vecvec<<<Gr, bin_rows>>>(voice, rows, cols, offset, bins_len, bins, bin_rows, bin_cols, mel_energies_out, mel_stride, htk_mode);
}

//functions for extract window
__global__
static void _F_cuda_process_gauss_val(float *val, int32_cuda len){
	int32_cuda idx = threadIdx.x + blockIdx.x * blockDim.x;
	int32_cuda stride = gridDim.x * blockDim.x;
	while(idx < len){
		*(val + idx) = sqrtf(-2 * logf(*(val + idx))) * cospif(*(val + idx + len) * 2);
		idx += stride;
	}
}

__host__
static void _F_process_gauss_val(float *val, int32_cuda len, const int32_cuda Gr, const int32_cuda Bl){
	_F_cuda_process_gauss_val<<<Gr,Bl>>>(val,len);
}

__global__ 
static void _D_cuda_process_gauss_val(double *val, int32_cuda len){
	int32_cuda idx = threadIdx.x + blockIdx.x * blockDim.x;
	int32_cuda stride = gridDim.x * blockDim.x;
	while(idx < len){
		*(val + idx) = sqrt(-2 * log(*(val + idx))) * cospi(*(val + idx + len) * 2);
		idx += stride;
	}
}

__host__
static void _D_process_gauss_val(double *val, int32_cuda len, const int32_cuda Gr, const int32_cuda Bl){
	_D_cuda_process_gauss_val<<<Gr,Bl>>>(val, len);
}

//block = numframes, thread = cols
template<typename Real>
__global__
static void _cuda_dither(Real *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda matrix_stride, Real dither_value, float *gauss_val){
		
	int32_cuda row = blockIdx.x;
	int32_cuda col = threadIdx.x;
	if(row > rows || col > cols) return;
	while(row < rows){
		for(int32_cuda i = col; i < cols; i += blockDim.x)
			*(waveform + row * matrix_stride + i) += dither_value * (*(gauss_val + row * cols + i));
		row += gridDim.x;
	}
}

template<typename Real>
__host__
static void _my_cuda_dither(Real *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda matrix_stride, Real dither_value, const int32_cuda Gr, const int32_cuda Bl){
	curandGenerator_t gen;
	clock_t seed = clock();
	float *gauss_val;
	CUDA_CALL(cudaMalloc((void **)&gauss_val, sizeof(float) * rows * cols));	
	CURAND_CALL(curandCreateGenerator(&gen, CURAND_RNG_PSEUDO_DEFAULT));
	CURAND_CALL(curandSetPseudoRandomGeneratorSeed(gen, seed));
	CURAND_CALL(curandGenerateNormal(gen, gauss_val, rows * cols, 0, 1));
	_cuda_dither<<<Gr,Bl>>>(waveform, rows, cols, matrix_stride, dither_value, gauss_val);
	CURAND_CALL(curandDestroyGenerator(gen));
	CUDA_CALL(cudaFree(gauss_val));
}

//block = numframes
template<typename Real>
__global__
static void _cuda_preemphasize(Real *waveform, int32_cuda rows, int32_cuda cols, int32_cuda matrix_stride, Real coeff){
	int32_cuda idx = blockIdx.x;
	if(coeff == 0.0 ||  idx >= rows) return;
	for (int32_cuda i = matrix_stride * idx + cols - 1; i > matrix_stride * idx; i--)
    	*(waveform + i) -= coeff * *(waveform + i - 1);
  	*(waveform + matrix_stride * idx) -= coeff * *(waveform + matrix_stride * idx);
}

template<typename Real>
__host__
static void _my_cuda_preemphasize(Real *waveform, int32_cuda rows, int32_cuda cols, int32_cuda matrix_stride, Real coeff, const int32_cuda Gr, const int32_cuda Bl){
	_cuda_preemphasize<<<Gr,Bl>>>(waveform, rows, cols, matrix_stride, coeff);
}

template<typename Real>
__global__
static void _cuda_wave_sum(const Real *waveform, int32_cuda rows, int32_cuda cols, int32_cuda matrix_stride, Real *wave_sum){
	int32_cuda row = blockIdx.x;
	int32_cuda stride = gridDim.x;
	if(row >= rows) return;
	while(row < rows){
		for(int32_cuda i = 0; i < cols; i++){
			*(wave_sum + row) = (*(wave_sum + row)) + (*(waveform + row * matrix_stride + i));
		}
		*(wave_sum + row) = (*(wave_sum + row)) / cols;
		row += stride;
	}
}

template<typename Real>
__host__
static void _my_cuda_wave_sum(const Real *waveform, int32_cuda rows, int32_cuda cols, int32_cuda matrix_stride, Real *wave_sum, const int32_cuda Gr, const int32_cuda Bl){
	_cuda_wave_sum<<<Gr,Bl>>>(waveform, rows, cols, matrix_stride, wave_sum);
	
}

//for wave mat elementwise multiple a window vector, same col length
template<typename Real>
__global__
static void _cuda_wave_mul(Real *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda stride, const Real *window){
	int row = blockIdx.x;
	for(int32_cuda col = threadIdx.x; col < cols; col += blockDim.x)
		*(waveform + row * stride + col) *= *(window + col);
}

template<typename Real>
__host__
static void _my_cuda_wave_mul(Real *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda stride, const Real *window, const int32_cuda Gr, const int32_cuda Bl){
	_cuda_wave_mul<<<Gr, Bl>>>(waveform, rows, cols, stride, window);
}


//seting the elements between cols and matrix_stride 0
//block = numframes, thread = elements
template<typename Real>
__global__
static void _cuda_set_zero(Real *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda matrix_stride){
	int32_cuda col = threadIdx.x;
	int32_cuda col_stride = blockDim.x;
	int32_cuda row = blockIdx.x;
	int32_cuda row_stride = gridDim.x;
	if(row >= rows || col >= matrix_stride - cols) return;
	while (row < rows){
		for(int32_cuda i = col; i < matrix_stride - cols; i += col_stride)
			*(waveform + i + cols + row * matrix_stride) = 0; 
		row += row_stride;
	}	
}

template<typename Real>
__host__
static void _my_cuda_set_zero(Real *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda matrix_stride, const int32_cuda Gr, const int32_cuda Bl){
	_cuda_set_zero<<<Gr,Bl>>>(waveform, rows, cols, matrix_stride);
}

//block = numframes, thread = element
template<typename Real>
__global__
static void _cuda_wave_dc_offset(Real *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda matrix_stride, const Real *wave_sum){
	int32_cuda row = blockIdx.x;
	int32_cuda row_stride = gridDim.x;
	int32_cuda col = threadIdx.x;
	int32_cuda col_stride = blockDim.x;
	if(row >= rows || col >= cols) return;
	while(row < rows){
		for(int32_cuda i = col; i < cols; i += col_stride)
			*(waveform + row * matrix_stride + i) = (*(waveform + row * matrix_stride + i)) - (*(wave_sum + row)); 
		row += row_stride;
	}

}

template<typename Real>
__host__
static void _my_cuda_wave_dc_offset(Real *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda matrix_stride, const Real *wave_sum, const int32_cuda Gr, const int32_cuda Bl){
	 _cuda_wave_dc_offset<<<Gr,Bl>>>(waveform, rows, cols, matrix_stride, wave_sum);
}

// For parallel computing log energy function
template<typename Real>
__global__ 
static void _get_element(Real *des, const Real *src, const int32_cuda rows, const int32_cuda cols, int32_cuda stride, int32_cuda bidx, int32_cuda acc, const Real lower_bound){
	if (blockIdx.x >= rows) return;
	int32_cuda tid = blockIdx.x;
	*(des + tid + bidx) = *(src + tid * stride + bidx + tid * acc) < lower_bound ? lower_bound : *(src + tid * stride + bidx + tid * acc);
}

__global__
static void _F_log(float *des){
	*(des + blockIdx.x) = logf(*(des + blockIdx.x));
}

__global__
static void _D_log(double *des){
	*(des + blockIdx.x) = log(*(des + blockIdx.x));
}

__host__
static void _F_log_energy(const float *src, int32_cuda rows, int32_cuda cols, int32_cuda stride, float *des){
	float alpha = 1.0f;
	float beta = 0.0f;
	float *tmp;
    CUDA_CALL(cudaMalloc((void **)&tmp, sizeof(float) * rows * rows));
	cublasHandle_t handle;
	cublasCreate(&handle);
	cublasSgemm_v2(handle, CUBLAS_OP_T, CUBLAS_OP_N, rows, rows, stride, &alpha, src, stride, src, stride, &beta, tmp, rows);
	_get_element<<<rows,1>>>(des, tmp, rows, rows, rows, 0, 1, numeric_limits<float>::min());
	_F_log<<<rows,1>>>(des);

	CUDA_CALL(cudaFree(tmp));
}

__host__
static void _D_log_energy(const double *src, int32_cuda rows, int32_cuda cols, int32_cuda stride, double *des){
	double alpha = 1.0f;
	double beta = 0.0f;
	double *tmp;
    CUDA_CALL(cudaMalloc((void **)&tmp, sizeof(double) * rows * stride));	
	cublasHandle_t handle;
	cublasCreate(&handle);
	cublasDgemm_v2(handle, CUBLAS_OP_T, CUBLAS_OP_N, rows, rows, cols, &alpha, src, cols, src, cols, &beta, tmp, rows);
	cublasDestroy(handle);
	_get_element<<<rows,1>>>(des, tmp, rows, rows, stride, 0, 1, numeric_limits<double>::min());
	_D_log<<<rows,1>>>(des);
	CUDA_CALL(cudaFree(tmp));
}

// functions for srfft
template<typename Real>
__device__
static void _swap(Real &a, Real &b){
	Real temp;
	temp = a;
	a = b;
	b = temp;
}

template<typename Real>
__device__
static void bitrp (Real *xreal, Real *ximag, int32_cuda n)
{
    // 位反转置换 Bit-reversal Permutation
	int32_cuda i, j, a, b, p;
	for (i = 1, p = 0; i < n; i *= 2)
	    p ++;
	for (i = 0; i < n; i ++){	
		a = i;
		b = 0;	
		for (j = 0; j < p; j ++){
			b = (b << 1) + (a & 1);    // b = b * 2 + a % 2;
			a >>= 1;        // a = a / 2;
		}
		if ( b > i){
			_swap (*(xreal + i), *(xreal + b));
			_swap (*(ximag + i), *(ximag + b));
		}
	}
}

template<typename Real>
__device__
static void FFT(Real *xreal, Real *ximag, Real *wreal, Real *wimag, const int32_cuda n)
{
    // 快速傅立叶变换，将复数 x 变换后仍保存在 x 中，xreal, ximag 分别是 x 的实部和虚部
	Real treal, timag, ureal, uimag;
	int32_cuda m, k, j, t, index1, index2;
	bitrp (xreal, ximag, n);
	for (m = 2; m <= n; m *= 2){
		for (k = 0; k < n; k += m){
			for (j = 0; j < m / 2; j ++){
				index1 = k + j;	
				index2 = index1 + m / 2;
				t = n * j / m;    // 旋转因子 w 的实部在 wreal [] 中的下标为 t
				treal = *(wreal + t) * *(xreal + index2) - *(wimag + t) * *(ximag + index2);
				timag = *(wreal + t) * *(ximag + index2) + *(wimag + t) * *(xreal + index2);
				ureal = *(xreal + index1);
				uimag = *(ximag + index1);
				*(xreal + index1) = ureal + treal;
				*(ximag + index1) = uimag + timag;	
				*(xreal + index2) = ureal - treal;
				*(ximag + index2) = uimag - timag;
			}
		}
	}
}

template<typename Real>
__device__
static void _cuda_memcpy(Real *des, Real *src, int32_cuda n){
	for(int32_cuda i = 0; i < n; i++)
		*(des + i) = *(src + i);
}

template<typename Real>
__device__
inline void _complexAddProduct(const Real &a_re, const Real &a_im, const Real &b_re, const Real &b_im, Real *c_re, Real *c_im ){
	*c_re += b_re*a_re - b_im*a_im;
	*c_im += b_re*a_im + b_im*a_re;
}

template<typename Real>
__device__
inline void _complexMul(const Real &a_re, const Real &a_im, Real *b_re, Real *b_im){
		Real tmp_re = (*b_re * a_re) - (*b_im * a_im);
	   *b_im = *b_re * a_im + (*b_im * a_re);
	   *b_re = tmp_re;
}

template<typename Real>
__device__
inline void _complexImExp(Real x, Real *a_re, Real *a_im){
	*a_re = cos(x);
	*a_im = sin(x);
}

template<typename Real>
__device__
static void _FFT_trans(Real *data, int32_cuda stride){
	int32_cuda N = stride, N2 = N/2;
  	Real rootN_re, rootN_im;  // exp(-2pi/N), forward; exp(2pi/N), backward
  int32_cuda forward_sign = -1;
  _complexImExp(static_cast<Real>(2.0 * PI/N *forward_sign), &rootN_re, &rootN_im);
  Real kN_re = -forward_sign, kN_im = 0.0;  // exp(-2pik/N), forward; exp(-2pik/N), backward
  // kN starts out as 1.0 for forward algorithm but -1.0 for backward.
  for (int32_cuda k = 1; 2*k <= N2; k++) {
    _complexMul(rootN_re, rootN_im, &kN_re, &kN_im);

    Real Ck_re, Ck_im, Dk_re, Dk_im;
    // C_k = 1/2 (B_k + B_{N/2 - k}^*) :
    Ck_re = 0.5 * (data[2*k] + data[N - 2*k]);
    Ck_im = 0.5 * (data[2*k + 1] - data[N - 2*k + 1]);
    // re(D_k)= 1/2 (im(B_k) + im(B_{N/2-k})):
    Dk_re = 0.5 * (data[2*k + 1] + data[N - 2*k + 1]);
    // im(D_k) = -1/2 (re(B_k) - re(B_{N/2-k}))
    Dk_im =-0.5 * (data[2*k] - data[N - 2*k]);
    // A_k = C_k + 1^(k/N) D_k:
    data[2*k] = Ck_re;  // A_k <-- C_k
    data[2*k+1] = Ck_im;
    // now A_k += D_k 1^(k/N)
    _complexAddProduct(Dk_re, Dk_im, kN_re, kN_im, &(data[2*k]), &(data[2*k+1]));

    int32_cuda kdash = N2 - k;
    if (kdash != k) {
      // Next we handle the index k' = N/2 - k.  This is necessary
      // to do now, to avoid invalidating data that we will later need.
      // The quantities C_{k'} and D_{k'} are just the conjugates of C_k
      // and D_k, so the equations are simple modifications of the above,
      // replacing Ck_im and Dk_im with their negatives.
      data[2*kdash] = Ck_re;  // A_k' <-- C_k'
      data[2*kdash+1] = -Ck_im;
      // now A_k' += D_k' 1^(k'/N)
      // We use 1^(k'/N) = 1^((N/2 - k) / N) = 1^(1/2) 1^(-k/N) = -1 * (1^(k/N))^*
      // so it's the same as 1^(k/N) but with the real part negated.
      _complexAddProduct(Dk_re, -Dk_im, -kN_re, kN_im, &(data[2*kdash]), &(data[2*kdash+1]));
    }
  }

  {  // Now handle k = 0.
    // In simple terms: after the complex fft, data[0] becomes the sum of real
    // parts input[0], input[2]... and data[1] becomes the sum of imaginary
    // pats input[1], input[3]...
    // "zeroth" [A_0] is just the sum of input[0]+input[1]+input[2]..
    // and "n2th" [A_{N/2}] is input[0]-input[1]+input[2]... .
    Real zeroth = data[0] + data[1],
        n2th = data[0] - data[1];
    data[0] = zeroth;
    data[1] = n2th; 
  }
}

//global function for fft
//1. change array form (first part: real, second part: image)
//2. perform fft
//3. change back
template<typename Real>
__global__
static void _cuda_srfft(Real *wave, int32_cuda rows, int32_cuda stride, Real *temp_buffer, Real *wreal, Real *wimag){
	if(blockIdx.x >= rows) return;
	int32_cuda N = stride / 2;
	int32_cuda tid = blockIdx.x;
	for(int32_cuda i = 0; i < N; i++){
		*(wave + tid * stride + i) = *(wave + tid * stride + i * 2);
		*(temp_buffer + tid * stride + i) = *(wave + tid * stride + i * 2 + 1);
	}
	_cuda_memcpy(wave + tid * stride + N, temp_buffer + tid * stride, N);
	FFT(wave + tid * stride, wave + tid * stride + N, wreal, wimag, N);
	_cuda_memcpy(temp_buffer + tid * stride, wave + tid * stride + N, N);

	for(int32_cuda i = N - 1; i > 0; i--){
		*(wave + tid * stride + i * 2) = *(wave + tid * stride + i);
		*(wave + tid * stride + i * 2 + 1) = *(temp_buffer + tid * stride + i);
	}
	*(wave + tid * stride + 1) = *(temp_buffer + tid * stride);
	_FFT_trans(wave + tid * stride, stride);
}

template<typename Real>
__host__
static void _my_cuda_srfft(Real *wave, int32_cuda rows, int32_cuda stride, Real *temp_buffer){
	Real *wreal, *wimag, *dev_wreal, *dev_wimag, arg, treal, timag;
	int32_cuda n = stride / 2;
	wreal = (Real *)malloc(sizeof(Real) * n);
	wimag = (Real *)malloc(sizeof(Real) * n);
	arg = - 2.0 * PI / n;
	treal = cos (arg);
	timag = sin (arg);
	*wreal = 1.0;
	*wimag = 0.0;
	
	for (int32_cuda j = 1; j < n / 2; j ++)
	{
		*(wreal + j) = *(wreal + j - 1) * treal - *(wimag + j - 1) * timag;
		*(wimag + j) = *(wreal + j - 1) * timag + *(wimag + j - 1) * treal;
	}
	CUDA_CALL(cudaMalloc((void **)&dev_wreal, sizeof(Real) * n));
	CUDA_CALL(cudaMalloc((void **)&dev_wimag, sizeof(Real) * n));
	CUDA_CALL(cudaMemcpy(dev_wreal, wreal, sizeof(Real) * n, cudaMemcpyHostToDevice));
	CUDA_CALL(cudaMemcpy(dev_wimag, wimag, sizeof(Real) * n, cudaMemcpyHostToDevice));
	_cuda_srfft<<<rows, 1>>>(wave, rows, stride, temp_buffer, dev_wreal, dev_wimag);
	

	free(wreal);
	free(wimag);
	CUDA_CALL(cudaFree(dev_wreal));
	CUDA_CALL(cudaFree(dev_wimag));
}

//compute power spectrum of FFT
template<typename Real>
__global__
static void _compute_power(Real *waveform, int32_cuda rows, int32_cuda cols, int32_cuda stride){
	int32_cuda half_dim = stride / 2;
	int32_cuda row = blockIdx.x; 
	Real first_energy = *(waveform + row * stride) * *(waveform + row * stride),
      last_energy = *(waveform + row * stride + 1) * *(waveform + row * stride + 1);  // handle this special case
  for (int32_cuda i = 1; i < half_dim; i++) {
    Real real = *(waveform + row * stride + i * 2), im = *(waveform + row * stride + i * 2 + 1);
    *(waveform + row * stride + i) = real * real + im * im;
  }
  *(waveform + row * stride) = first_energy;
  *(waveform + row * stride + half_dim) = last_energy;
}
	
template<typename Real>
__host__
static void _my_cuda_compute_power(Real *waveform, int32_cuda rows, int32_cuda cols, int32_cuda stride){
	_compute_power<<<rows, 1>>>(waveform, rows, cols, stride);
}

//set 0-th energy 
template<typename Real>
__global__
static void _set_energy(Real *des, const int32_cuda stride, const Real energy_floor, const Real log_energy_floor, const Real *src){
	int32_cuda tid = blockIdx.x;
	Real log_energy = *(src + tid);
	if(energy_floor > 0.0 && log_energy < log_energy_floor)
		log_energy = log_energy_floor;
	*(des + tid * stride) = log_energy;
}

template<typename Real>
__host__
static void _my_cuda_set_energy(Real *des, const int32_cuda rows, const int32_cuda stride, const Real energy_floor, const Real log_energy_floor, const Real *src){
	_set_energy<<<rows, 1>>>(des, stride, energy_floor, log_energy_floor, src);
}

template <typename Real>
__host__
static void _my_cuda_addvec2(dim3 Gr, dim3 Bl, Real *A, const Real *x, int32_cuda dim, Real alpha)
{
	_my_addvec2<<<Gr, Bl>>>(A, x, dim, alpha);
}

template <typename Real>
__host__
static void _my_cuda_addvec3(dim3 Gr, dim3 Bl, Real *A, int32_cuda numA, const Real *x, MatrixDim d, Real alpha)
{
	_my_addvec3<<<Gr, Bl>>>(A, numA, x, d, alpha);
}

template <typename Real>
__host__
static void _my_cuda_compute_posterior(int32_cuda Gr, int32_cuda Bl, Real *loglikes, MatrixDim d_log, Real min_post)
{
	_compute_posterior<<<Gr, Bl>>>(loglikes, d_log, min_post);
}

void _F_my_cuda_compute_posterior(float *loglikes, MatrixDim d_log, float min_post, int32_cuda Gr, int32_cuda Bl)
{
	_my_cuda_compute_posterior(Gr, Bl, loglikes, d_log, min_post);
}

void _D_my_cuda_compute_posterior(double *loglikes, MatrixDim d_log, double min_post, int32_cuda Gr, int32_cuda Bl)
{
	_my_cuda_compute_posterior(Gr, Bl, loglikes, d_log, min_post);
}

//define in my-cuda-function-kernel-ansi.h
void _F_my_cuda_compute_fft(float *data, int32_cuda dim)
{
	_my_cuda_compute_fft(data, dim);
}

void _D_my_cuda_compute_fft(double *data, int32_cuda dim)
{
	_my_cuda_compute_fft(data, dim);
}

void _F_my_cuda_gmm_select(int32_cuda Gr, int32_cuda Bl, const float *data, MatrixDim d, int32_cuda num_gselect, int32_cuda *gmm_out)
{
	_my_cuda_gmm_select(Gr, Bl, data, d, num_gselect, gmm_out);
}

void _D_my_cuda_gmm_select(int32_cuda Gr, int32_cuda Bl, const double *data, MatrixDim d, int32_cuda num_gselect, int32_cuda *gmm_out)
{
	_my_cuda_gmm_select(Gr, Bl, data, d, num_gselect, gmm_out);
}

void _F_my_cuda_add_mat_vec(const float *gamma, const float *Sigma_inv_M_, float *data, int32_cuda gamma_size, int32_cuda Sigma_row, int32_cuda Sigma_col){
    _add_mat_vec(gamma, Sigma_inv_M_, data, gamma_size, Sigma_row, Sigma_col);
}

void _D_my_cuda_add_mat_vec(const double *gamma, const double *Sigma_inv_M_, double *data, int32_cuda gamma_size, int32_cuda Sigma_row, int32_cuda Sigma_col){
    _add_mat_vec(gamma, Sigma_inv_M_, data, gamma_size, Sigma_row, Sigma_col);
}

void _F_my_cuda_parallel_memcpy(float *des, float *src, int32_cuda sizes, int32_cuda rows, int32_cuda cols, int32_cuda stride, int32_cuda Gr, int32_cuda Bl){
	_my_parallel_memcpy(des, src, sizes, rows, cols, stride, Gr, Bl);	
}

void _D_my_cuda_parallel_memcpy(double *des, double *src, int32_cuda sizes, int32_cuda rows, int32_cuda cols, int32_cuda stride, int32_cuda Gr, int32_cuda Bl){
	_my_parallel_memcpy(des, src, sizes, rows, cols, stride, Gr, Bl);
}

void _F_my_cuda_vec_memcpy(const float *src, int32_cuda len, int32_cuda blank, float *des, int32_cuda rows, int32_cuda cols, int32_cuda stride, int32_cuda Gr, int32_cuda Bl){
	_cuda_vec_memcpy(src, len, blank, des, rows, cols, stride, Gr, Bl);
}

void _D_my_cuda_vec_memcpy(const double *src, int32_cuda len, int32_cuda blank, double *des, int32_cuda rows, int32_cuda cols, int32_cuda stride, int32_cuda Gr, int32_cuda Bl){
	_cuda_vec_memcpy(src, len, blank, des, rows, cols, stride, Gr, Bl);
}

void _F_my_cuda_parallel_mel_vecvec(const float *voice, const int32_cuda rows, const int32_cuda cols, const int32_cuda *offset, const int32_cuda *bins_len, const float *bins, const int32_cuda bin_rows, const int32_cuda bin_cols, float *mel_energies_out, const int32_cuda mel_stride, int32_cuda htk_mode, int32_cuda Gr){
	_my_cuda_parallel_mel_vecvec(voice, rows, cols, offset, bins_len, bins, bin_rows, bin_cols, mel_energies_out, mel_stride, htk_mode, Gr);
} 

void _D_my_cuda_parallel_mel_vecvec(const double *voice, const int32_cuda rows, const int32_cuda cols, const int32_cuda *offset, const int32_cuda *bins_len, const double *bins, const int32_cuda bin_rows, const int32_cuda bin_cols, double *mel_energies_out, const int32_cuda mel_stride, int32_cuda htk_mode, int32_cuda Gr){
	_my_cuda_parallel_mel_vecvec(voice, rows, cols, offset, bins_len, bins, bin_rows, bin_cols, mel_energies_out, mel_stride, htk_mode, Gr);
} 

// functions for extracting window
void _F_my_cuda_process_gauss_val(float *val, int32_cuda len, const int32_cuda Gr, const int32_cuda Bl){
	_F_process_gauss_val(val, len, Gr, Bl);
}

void _F_my_cuda_scale_linear(float *A, int32_cuda dim, float alpha, int32_cuda Gr, int32_cuda Bl)
{
	_my_cuda_scale_linear(Gr, Bl, A, dim, alpha);
}

void _D_my_cuda_scale_linear(double *A, int32_cuda dim, double alpha, int32_cuda Gr, int32_cuda Bl)
{
	_my_cuda_scale_linear(Gr, Bl, A, dim, alpha);
}

void _F_my_cuda_addvec2(float *A, const float *x, int32_cuda dim, float alpha, dim3 Gr, dim3 Bl)
{
	_my_cuda_addvec2(Gr, Bl, A, x, dim, alpha);
}

void _D_my_cuda_addvec2(double *A, const double *x, int32_cuda dim, double alpha, dim3 Gr, dim3 Bl)
{
	_my_cuda_addvec2(Gr, Bl, A, x, dim, alpha);
}

void _F_my_cuda_addvec3(float *A, int32_cuda numA, const float *x, MatrixDim d, float alpha, dim3 Gr, dim3 Bl)
{
	_my_cuda_addvec3(Gr, Bl, A, numA, x, d, alpha);
}

void _D_my_cuda_addvec3(double *A, int32_cuda numA, const double *x, MatrixDim d, double alpha, dim3 Gr, dim3 Bl)
{
	_my_cuda_addvec3(Gr, Bl, A, numA, x, d, alpha);
}

void _F_my_cuda_MatVecVec(const float *A, const float *B, MatrixDim dA, MatrixDim dB, float *x, int32_cuda Gr, int32_cuda Bl)
{
	_my_cuda_MatVecVec(A, B, dA, dB, x, Gr, Bl);
}

void _D_my_cuda_MatVecVec(const double *A, const double *B, MatrixDim dA, MatrixDim dB, double *x, int32_cuda Gr, int32_cuda Bl)
{
	_my_cuda_MatVecVec(A, B, dA, dB, x, Gr, Bl);
}

void _D_my_cuda_process_gauss_val(double *val, int32_cuda len, const int32_cuda Gr, const int32_cuda Bl){
	_D_process_gauss_val(val, len, Gr, Bl);
}


void _F_my_cuda_dither(float *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda matrix_stride, float dither_value, const int32_cuda Gr, const int32_cuda Bl){
	_my_cuda_dither(waveform, rows, cols, matrix_stride, dither_value, Gr, Bl);
}

void _D_my_cuda_dither(double *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda matrix_stride, double dither_value, const int32_cuda Gr, const int32_cuda Bl){
	_my_cuda_dither(waveform, rows, cols, matrix_stride, dither_value, Gr, Bl);
}

void _F_my_cuda_preemphasize(float *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda matrix_stride, const float coeff, const int32_cuda Gr, const int32_cuda Bl){
	_my_cuda_preemphasize(waveform, rows, cols, matrix_stride, coeff, Gr, Bl);
}

void _D_my_cuda_preemphasize(double *waveform,const int32_cuda rows, const int32_cuda cols, const int32_cuda matrix_stride, const double coeff, const int32_cuda Gr, const int32_cuda Bl){
	_my_cuda_preemphasize(waveform, rows, cols, matrix_stride, coeff, Gr, Bl);
}

void _F_my_cuda_wave_sum(const float *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda matrix_stride, float *wave_sum, const int32_cuda Gr, const int32_cuda Bl){
	_my_cuda_wave_sum(waveform, rows, cols, matrix_stride, wave_sum, Gr, Bl);
}

void _D_my_cuda_wave_sum(const double *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda matrix_stride, double *wave_sum, const int32_cuda Gr, const int32_cuda Bl){
	_my_cuda_wave_sum(waveform, rows, cols, matrix_stride, wave_sum, Gr, Bl);
}

void _F_my_cuda_wave_mul(float *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda stride, const float *window, const int32_cuda Gr, const int32_cuda Bl){
	_my_cuda_wave_mul(waveform, rows, cols, stride, window, Gr, Bl);
}

void _D_my_cuda_wave_mul(double *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda stride, const double *window, const int32_cuda Gr, const int32_cuda Bl){
	_my_cuda_wave_mul(waveform, rows, cols, stride, window, Gr, Bl);
}

void _F_my_cuda_LogLikelihoodsPreselect(const int32_cuda *gselect, int32_cuda gselect_rows, int32_cuda gselect_cols, const float *features, MatrixDim d_features, const float *gconsts_, int32_cuda dim_gconsts, const float *means_invcovars_, MatrixDim d_means, const float *data_sqs, const float *inv_covars_, int32_cuda spdim, float *loglikes, MatrixDim d_loglikes, dim3 Gr, dim3 Bl)
{
	_my_cuda_LogLikelihoodsPreselect(gselect, gselect_rows, gselect_cols, features, d_features, gconsts_, dim_gconsts, means_invcovars_, d_means, data_sqs, inv_covars_, spdim, loglikes, d_loglikes, Gr, Bl);
}

void _D_my_cuda_LogLikelihoodsPreselect(const int32_cuda *gselect, int32_cuda gselect_rows, int32_cuda gselect_cols, const double *features, MatrixDim d_features, const double *gconsts_, int32_cuda dim_gconsts, const double *means_invcovars_, MatrixDim d_means, const double *data_sqs, const double *inv_covars_, int32_cuda spdim, double *loglikes, MatrixDim d_loglikes, dim3 Gr, dim3 Bl)
{
	_my_cuda_LogLikelihoodsPreselect(gselect, gselect_rows, gselect_cols, features, d_features, gconsts_, dim_gconsts, means_invcovars_, d_means, data_sqs, inv_covars_, spdim, loglikes, d_loglikes, Gr, Bl);
}

void _F_my_cuda_scale_diag_numsp(float *A, int32_cuda nums, MatrixDim d, float alpha, dim3 Gr, dim3 Bl)
{
	_my_cuda_scale_diag_numsp(A, nums, d, alpha, Gr, Bl);
}

void _D_my_cuda_scale_diag_numsp(double *A, int32_cuda nums, MatrixDim d, double alpha, dim3 Gr, dim3 Bl)
{
	_my_cuda_scale_diag_numsp(A, nums, d, alpha, Gr, Bl);
}

void _F_my_cuda_MatApplySoftMax(float *data, MatrixDim d, int32_cuda Gr, int32_cuda Bl)
{
	_my_cuda_MatApplySoftMax(data, d, Gr, Bl);
}

void _D_my_cuda_MatApplySoftMax(double *data, MatrixDim d, int32_cuda Gr, int32_cuda Bl)
{
	_my_cuda_MatApplySoftMax(data, d, Gr, Bl);
}

void _F_my_cuda_AddMatColsToVec(const float *mat, MatrixDim d, float *vec, int32_cuda Gr, int32_cuda Bl)
{
	_my_cuda_AddMatColsToVec(mat, d, vec, Gr, Bl);
}

void _F_my_cuda_set_zero(float *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda matrix_stride, const int32_cuda Gr, const int32_cuda Bl){
	_my_cuda_set_zero(waveform, rows, cols, matrix_stride, Gr, Bl);
}

void _D_my_cuda_AddMatColsToVec(const double *mat, MatrixDim d, double *vec, int32_cuda Gr, int32_cuda Bl)
{
	_my_cuda_AddMatColsToVec(mat, d, vec, Gr, Bl);
}

void _D_my_cuda_set_zero(double *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda matrix_stride, const int32_cuda Gr, const int32_cuda Bl){
	_my_cuda_set_zero(waveform, rows, cols, matrix_stride, Gr, Bl);
}

void _F_my_cuda_wave_dc_offset(float *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda matrix_stride, const float *wave_sum, const int32_cuda Gr, const int32_cuda Bl){
	 _my_cuda_wave_dc_offset(waveform, rows, cols, matrix_stride, wave_sum, Gr, Bl);
}

void _D_my_cuda_wave_dc_offset(double *waveform, const int32_cuda rows, const int32_cuda cols, const int32_cuda matrix_stride, const double *wave_sum, const int32_cuda Gr, const int32_cuda Bl){
	 _my_cuda_wave_dc_offset(waveform, rows, cols, matrix_stride, wave_sum, Gr, Bl);
}

void _F_my_cuda_log_energy(const float *src, int32_cuda rows, int32_cuda cols, int32_cuda stride, float *des){
	_F_log_energy(src, rows, cols, stride, des);
}

void _D_my_cuda_log_energy(const double *src, int32_cuda rows, int32_cuda cols, int32_cuda stride, double *des){
	_D_log_energy(src, rows, cols, stride, des);
}

void _F_my_cuda_srfft(float *wave, int32_cuda rows, int32_cuda stride, float *temp_buffer){
	_my_cuda_srfft(wave, rows, stride, temp_buffer);
}

void _D_my_cuda_srfft(double *wave, int32_cuda rows, int32_cuda stride, double *temp_buffer){
	_my_cuda_srfft(wave, rows, stride, temp_buffer);
}

void _F_my_cuda_compute_power(float *waveform, int32_cuda rows, int32_cuda cols, int32_cuda stride){
	_my_cuda_compute_power(waveform, rows, cols, stride);
}

void _D_my_cuda_compute_power(double *waveform, int32_cuda rows, int32_cuda cols, int32_cuda stride){
	_my_cuda_compute_power(waveform, rows, cols, stride);
}

void _F_my_cuda_set_energy(float *des, const int32_cuda rows, const int32_cuda stride, const float energy_floor, const float log_energy_floor, const float *src){
	_my_cuda_set_energy(des, rows, stride, energy_floor, log_energy_floor, src);
}

void _D_my_cuda_set_energy(double *des, const int32_cuda rows, const int32_cuda stride, const double energy_floor, const double log_energy_floor, const double *src){
	_my_cuda_set_energy(des, rows, stride, energy_floor, log_energy_floor, src);
}
