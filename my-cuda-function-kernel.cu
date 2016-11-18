#include <cuda_runtime.h>
#include <cfloat>
#include <cufft.h>
#include <cuda.h>
#include <stdio.h>
#include <stdlib.h>
#include "my-cuda-function-kernel-ansi.h"
#include <algorithm>

#define CUDA_CALL(ret) \
{\
  if((ret) != cudaSuccess) { \
  printf("Error at %s:%d\n", __FILE__, __LINE__); \
  printf("Error code %s", cudaGetErrorString(ret)); \
  exit(-1); \
  } \
  cudaThreadSynchronize(); \
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
static void _gmm_select(Real *data, MatrixDim d, Real *copydata, MatrixDim c_d, int32_cuda num_gselect, int32_cuda *gmm_selected)
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
static void _my_cuda_gmm_select(int32_cuda Gr, int32_cuda Bl, Real *data, MatrixDim d, int32_cuda num_gselect, int32_cuda *gmm_out)
{
	int32_cuda *selected_gauss;
	Real *copydata;
	size_t pitch;
	MatrixDim c_d;
	c_d.rows = d.rows;
	c_d.cols = d.cols;
	CUDA_CALL(cudaMallocPitch((void **)&copydata, &pitch, d.cols * sizeof(Real), d.rows));
	c_d.stride = pitch / sizeof(Real);
	CUDA_CALL(cudaMemcpy2D(copydata, c_d.stride * sizeof(Real), data, d.stride * sizeof(Real), d.cols * sizeof(Real), d.rows, cudaMemcpyDeviceToDevice));
	CUDA_CALL(cudaMalloc((void **)&selected_gauss, d.rows * num_gselect * sizeof(int32_cuda)));
	cudaDeviceSynchronize();
//	printdata<<<1,1>>>(copydata, c_d);
	_gmm_select<<<Gr, Bl>>>(data, d, copydata, c_d, num_gselect, selected_gauss);
	CUDA_CALL(cudaMemcpy(gmm_out, selected_gauss, d.rows * num_gselect * sizeof(int32_cuda), cudaMemcpyDeviceToHost));
	CUDA_CALL(cudaFree(selected_gauss));
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


//define in my-cuda-function-kernel-ansi.h
void _F_my_cuda_compute_fft(float *data, int32_cuda dim)
{
	_my_cuda_compute_fft(data, dim);
}

void _D_my_cuda_compute_fft(double *data, int32_cuda dim)
{
	_my_cuda_compute_fft(data, dim);
}

void _F_my_cuda_gmm_select(int32_cuda Gr, int32_cuda Bl, float *data, MatrixDim d, int32_cuda num_gselect, int32_cuda *gmm_out)
{
	_my_cuda_gmm_select(Gr, Bl, data, d, num_gselect, gmm_out);
}

void _D_my_cuda_gmm_select(int32_cuda Gr, int32_cuda Bl, double *data, MatrixDim d, int32_cuda num_gselect, int32_cuda *gmm_out)
{
	_my_cuda_gmm_select(Gr, Bl, data, d, num_gselect, gmm_out);
}

