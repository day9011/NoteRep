#if HAVE_CUDA == 1
#include <iostream>
#include "cudalib/my-cuda-data-struct.h"
#include "cudalib/my-cuda-function-kernel.h"
using namespace std;

//CuMatrixInt
void CuMatrixInt::Resize(int32_cuda rows, int32_cuda cols)
{
	KALDI_ASSERT(rows > 0 && cols > 0);
	if (CuDevice::Instantiate().Enabled())
	{
		if (this->rows_ != 0)
			this->Destroy();
		int32_cuda num_bytes = rows * cols * sizeof(int32_cuda);
		this->data_ = static_cast<int32_cuda*>(CuDevice::Instantiate().Malloc(num_bytes));
		this->rows_ = rows;
		this->cols_ = cols;
	}
}

void CuMatrixInt::Destroy()
{
	if (CuDevice::Instantiate().Enabled())
	{
		if (this->data_ != NULL)
			CuDevice::Instantiate().Free(this->data_);
		this->rows_ = 0;
		this->cols_ = 0;
	}
}

//CuMatrixIntBase
void CuMatrixIntBase::CopyFromMat(const MatrixIntBase &mat)
{
	KALDI_ASSERT(mat.NumRows() == this->rows_ && mat.NumCols() == this->cols_);
	if (CuDevice::Instantiate().Enabled())
	{
		my_time t;
		t.start();
		KALDI_LOG << "start copy data of MatrixInt";
		CU_SAFE_CALL(cudaMemcpy(this->data_, mat.data_, this->rows_ * this->cols_ * sizeof(int32_cuda), cudaMemcpyHostToDevice));
		t.end();
		KALDI_LOG << "Copy MatrixInt data to CuMatrixInt time: " << t.used_time() << "ms";
	}
}

void CuMatrixIntBase::CopyFromMat(const CuMatrixIntBase &mat)
{
	KALDI_ASSERT(mat.NumRows() == this->rows_ && mat.NumCols() == this->cols_);
	if (CuDevice::Instantiate().Enabled())
	{
		my_time t;
		t.start();
		KALDI_LOG << "start copy data of CuMatrixInt";
		CU_SAFE_CALL(cudaMemcpy(this->data_, mat.data_, this->rows_ * this->cols_ * sizeof(int32_cuda), cudaMemcpyDeviceToDevice));
		t.end();
		KALDI_LOG << "Copy CuMatrixInt data to CuMatrixInt time: " << t.used_time() << "ms";
	}
}

//MatrixInt data struct
void MatrixIntBase::CopyFromMat(const CuMatrixIntBase &mat)
{
	KALDI_ASSERT(mat.NumRows() == this->rows_ && mat.NumCols() == this->cols_);
	if (CuDevice::Instantiate().Enabled())
	{
		my_time t;
		t.start();
		KALDI_LOG << "start copy data of CuMatrixInt";
		CU_SAFE_CALL(cudaMemcpy(this->data_, mat.data_, this->rows_ * this->cols_ * sizeof(int32_cuda), cudaMemcpyDeviceToHost));
		t.end();
		KALDI_LOG << "Copy CuMatrixInt data to MatrixInt time: " << t.used_time() << "ms";
	}
}

void MatrixIntBase::CopyFromMat(const MatrixIntBase &mat)
{
	KALDI_ASSERT(mat.NumRows() == this->rows_ && mat.NumCols() == this->cols_);
	my_time t;
	t.start();
	KALDI_LOG << "start copy data of MatrixInt";
	for (int i = 0; i < this->rows_; i++)
		for (int j = 0; j < this->cols_; j++)
			this->data_[i * this->cols_ + j] = mat.data_[i * this->cols_ + j];
	t.end();
	KALDI_LOG << "Copy MatrixInt data to MatrixInt time: " << t.used_time() << "ms";
}

void MatrixInt::Resize(int32_cuda rows, int32_cuda cols)
{
	KALDI_ASSERT(rows > 0 && cols > 0);
	if (this->rows_ != 0)
		Destroy();
	int32_cuda num_bytes = rows * cols * sizeof(int32_cuda);
	this->data_ = (int32_cuda *)malloc(num_bytes);
	this->rows_ = rows;
	this->cols_ = cols;
}

void MatrixInt::Destroy()
{
	if (this->data_ != NULL)
		free(this->data_);
	this->rows_ = this->cols_ = 0;
}

//CuLinearMatrixBase data struct
template <typename Real>
void CuLinearMatrixBase<Real>::SetZero()
{
	if (CuDevice::Instantiate().Enabled())
	{
		int32_cuda num_bytes = DataNumBytes();
		CU_SAFE_CALL(cudaMemset(reinterpret_cast<void*>(this->data_), 0, static_cast<size_t>(num_bytes)));
	}
}

template <typename Real>
void CuLinearMatrixBase<Real>::CopyFromMat(const CuLinearMatrixBase<Real> &mat)
{
	if (CuDevice::Instantiate().Enabled())
	{
		KALDI_ASSERT(mat.NumCols() == this->cols_ && mat.NumRows() == this->rows_);
		my_time t;
		t.start();
		KALDI_LOG << "start copy data of CuLinearMatrix";
		CU_SAFE_CALL(cudaMemcpy(this->data_, mat.Data(), mat.DataNumBytes(), cudaMemcpyDeviceToDevice));
		t.end();
		KALDI_LOG << "Copy Matrix data to CuLinearMatrix time: " << t.used_time() << "ms";
	}
}

template <typename Real>
void CuLinearMatrixBase<Real>::CopyFromMat(const Matrix<Real> &mat)
{
	if (CuDevice::Instantiate().Enabled())
	{
		KALDI_ASSERT(mat.NumRows() == this->rows_ && mat.NumCols() == this->cols_);
		if (mat.NumCols() == 0 || mat.NumRows() == 0) return;
		my_time t;
		t.start();
		KALDI_LOG << "start copy data of Matrix";
		int32_cuda rows = mat.NumRows();
		int32_cuda cols = mat.NumCols();
		int32_cuda stride = mat.Stride();
	    CuMatrix<Real> cu_mat(rows, cols);
		CU_SAFE_CALL(cudaMemcpy(cu_mat.Data(), mat.Data(), sizeof(Real) * rows * cols, cudaMemcpyHostToDevice));
		my_cuda_parallel_memcpy(this->data_, const_cast<Real*>(cu_mat.Data()), 1, rows, cols, stride, 1024, cols);
		t.end();
		KALDI_LOG << "Parallel copy Matrix data to CuLinearMatrix time: " << t.used_time() << "ms";
	}
}

template <typename Real>
void CuLinearMatrixBase<Real>::CopyFromMat(const CuMatrix<Real> &mat)
{
	if (CuDevice::Instantiate().Enabled())
	{   
		KALDI_ASSERT(mat.NumRows() == this->rows_ && mat.NumCols() == this->cols_);
		if (mat.NumCols() == 0 || mat.NumRows() == 0) return;
		my_time t;
		t.start();
		KALDI_LOG << "start copy data of CuMatrix";
		int32_cuda rows = mat.NumRows();
		int32_cuda cols = mat.NumCols();
		int32_cuda stride = mat.Stride();
		my_cuda_parallel_memcpy(this->data_, const_cast<Real *>(mat.Data()), 1, rows, cols, stride, 1024, cols);
		t.end();
		KALDI_LOG << "Parallel copy CuMatrix data to CuLinearMatrix time: " << t.used_time() << "ms";
	}
}

//CuLinearMatrix data struct
template <typename Real>
void CuLinearMatrix<Real>::Resize(int32_cuda rows, int32_cuda cols)
{
	KALDI_ASSERT(rows > 0 || cols > 0);
	if (CuDevice::Instantiate().Enabled())
	{
		if (this->rows_ != 0)
			Destroy();
		int32_cuda num_bytes = rows * cols * sizeof(Real);
		this->data_ = static_cast<Real*>(CuDevice::Instantiate().Malloc(num_bytes));
		this->rows_ = rows;
		this->cols_ = cols;
	}
}

template <typename Real>
void CuLinearMatrix<Real>::Destroy()
{
	if (CuDevice::Instantiate().Enabled())
	{
		if (this->data_ != NULL)
			CuDevice::Instantiate().Free(this->data_);
		this->data_ = NULL;
		this->rows_ = 0;
		this->cols_ = 0;
	}
}


//CuNumMatrixBase data struct
template <typename Real>
void CuNumMatrixBase<Real>::SetZero()
{
	if (CuDevice::Instantiate().Enabled())
	{
		int32_cuda num_bytes = DataNumBytes();
		CU_SAFE_CALL(cudaMemset(reinterpret_cast<void*>(this->data_), 0, static_cast<size_t>(num_bytes)));
	}	
}

template <typename Real>
void CuNumMatrixBase<Real>::CopyFromMats(const std::vector<CuMatrix<Real> > &mats)
{
	if (CuDevice::Instantiate().Enabled())
	{
		KALDI_ASSERT(mats.size() == this->nums_ && mats[0].NumRows() == this->rows_ && mats[0].NumCols() == this->cols_);
		if (mats.size() == 0 || mats[0].NumRows() == 0) return;
		my_time t;
		t.start();
		KALDI_LOG << "start copy data of CuMatrix";
		int32_cuda sizes = mats.size();
		int32_cuda rows = mats[0].NumRows();
		int32_cuda cols = mats[0].NumCols();
		int32_cuda stride = mats[0].Stride();
		for (int32_cuda i = 0; i < sizes; i++)
		{
			my_cuda_parallel_memcpy(this->data_ + i * rows * cols, const_cast<Real *>(mats[i].Data()), 1, rows, cols, stride, 1024, cols);
		}
		t.end();
		KALDI_LOG << "Parallel copy CuMatrix data to CuNumMatrix time: " << t.used_time() << "ms";
	}
}

template <typename Real>
void CuNumMatrixBase<Real>::CopyFromMats(const std::vector<Matrix<Real> > &mats)
{
	if (CuDevice::Instantiate().Enabled())
	{
		KALDI_ASSERT(mats.size() == this->nums_ && mats[0].NumRows() == this->rows_ && mats[0].NumCols() == this->cols_);
		if (mats.size() == 0 || mats[0].NumRows() == 0) return;
		my_time t;
		t.start();
		KALDI_LOG << "start copy data of Matrix";
		int32_cuda sizes = mats.size();
		int32_cuda rows = mats[0].NumRows();
		int32_cuda cols = mats[0].NumCols();
		int32_cuda stride = mats[0].Stride();
		CuMatrix<Real> cu_mat(rows, cols);
		for (int32_cuda i = 0; i < sizes; i++)
		{
		CU_SAFE_CALL(cudaMemcpy(cu_mat.Data(), mats[i].Data(), sizeof(Real) * rows * cols, cudaMemcpyHostToDevice));
		my_cuda_parallel_memcpy(this->data_ + i * rows * cols, cu_mat.Data(), 1, rows, cols, stride, 1024, stride);
		}
		t.end();
		KALDI_LOG << "copy Matrix data to CuNumMatrix time: " << t.used_time() << "ms";
	}
}

template<typename Real>
void CuNumMatrixBase<Real>::Scale(Real alpha)
{
	if (CuDevice::Instantiate().Enabled())
	{
		KALDI_ASSERT(NumRows() > 0 && NumSizes() > 0 && NumCols() > 0);
		int rows = NumRows();
		int cols = NumCols();
		int size = NumSizes();
		int dimBlock(CU1DBLOCK);
		int dimGrid(n_blocks(size * rows * cols, CU1DBLOCK));
		my_cuda_scale_linear(this->data_, size * rows * cols, alpha, dimGrid, dimBlock);
	}
}

/*
template <typename Real>
void CuNumMatrixBase::GenerateSuperMatrix()
{
	if (CuDevice::Instantiate().Enabled())
	{
		KALDI_ASSERT(this->nums_ > 0 && this->rows_ > 0 && this->cols_ > 0 && this->data_ != NULL);
		my_time t;
		t.start();
		KALDI_LOG << "start generate super matrix";
		size_t num_bytes = DataNumBytes();
		this->super_data_ = static_cast<Real*>(CuDevice::Instantiate().Malloc(num_bytes));
		Real *temp_row_data = static_cast<Real*>(CuDevice::Instantiate())
	}
}
*/

//CuNumMatrix data struct
template <typename Real>
void CuNumMatrix<Real>::Resize(int32_cuda nums, int32_cuda rows, int32_cuda cols)
{
	KALDI_ASSERT(rows > 0 || cols > 0 || nums > 0);
	if (CuDevice::Instantiate().Enabled())
	{
		if (this->nums_ != 0)
			Destroy();
		int32_cuda num_bytes = nums * rows * cols * sizeof(Real);
		this->data_ = static_cast<Real*>(CuDevice::Instantiate().Malloc(num_bytes));
		this->nums_ = nums;
		this->rows_ = rows;
		this->cols_ = cols;
	}
}

template <typename Real>
void CuNumMatrix<Real>::Destroy()
{
	if (CuDevice::Instantiate().Enabled())
	{
		if (this->data_ != NULL)
			CuDevice::Instantiate().Free(this->data_);
		// if (this->super_data_ != NULL)
		//     CuDevice::Instantiate().Free(this->super_data_);
		// this->super_data_ = NULL:
		this->data_ = NULL;
		this->nums_ = 0;
		this->rows_ = 0;
		this->cols_ = 0;
	}
}

template <typename Real>
void CuNumMatrixBase<Real>::CopyFromMat(const CuNumMatrixBase<Real> &mat)
{
	if (CuDevice::Instantiate().Enabled())
	{
		KALDI_ASSERT(mat.NumSizes() == this->nums_ && mat.NumRows() == this->rows_ && mat.NumCols() == this->cols_);
		CU_SAFE_CALL(cudaMemcpy(this->data_, mat.Data(), mat.DataNumBytes(), cudaMemcpyDeviceToDevice));
	}
}



//CuNumPackedMatrix function
template <typename Real>
void CuNumPackedMatrix<Real>::SetZero()
{
	if (CuDevice::Instantiate().Enabled())
	{
		int32_cuda num_bytes = DataNumBytes();
		CU_SAFE_CALL(cudaMemset(reinterpret_cast<void*>(this->data_), 0, static_cast<size_t>(num_bytes)));
	}
}

template <typename Real>
void CuNumSpMatrix<Real>::Resize(int32_cuda nums, int32_cuda rows)
{
	KALDI_ASSERT(rows > 0 || nums > 0);
	if (CuDevice::Instantiate().Enabled())
	{
		if (this->nums_ != 0)
			Destroy();
		int32_cuda num_bytes = nums * (rows * (rows + 1) / 2) * sizeof(Real);
		this->data_ = static_cast<Real*>(CuDevice::Instantiate().Malloc(num_bytes));
		this->nums_ = nums;
		this->rows_ = rows;
		this->d_.stride = rows * (rows + 1) / 2;
		this->d_.rows = nums;
		this->d_.cols = rows;
	}
}

template <typename Real>
void CuNumSpMatrix<Real>::Destroy()
{
	if (CuDevice::Instantiate().Enabled())
	{
		if (this->data_ != NULL)
			CuDevice::Instantiate().Free(this->data_);
		this->data_ = NULL;
		this->nums_ = 0;
		this->rows_ = 0;
	}
}

template <typename Real>
void CuNumPackedMatrix<Real>::CopyFromMats(const std::vector<SpMatrix<Real> > &mats)
{
	if (CuDevice::Instantiate().Enabled())
	{
		KALDI_ASSERT(mats.size() == this->nums_ && mats[0].NumRows() == this->rows_);
		if (mats.size() == 0 || mats[0].NumRows() == 0) return;
		int32_cuda sizes = mats.size();
		int32_cuda rows = mats[0].NumRows();
		my_time t;
		t.start();
		KALDI_LOG << "start copy data of SpMatrix";
		
		for (int32_cuda i = 0; i < sizes; i++)
		{
			CU_SAFE_CALL(cudaMemcpy(this->data_ + i * rows * (rows + 1) / 2, mats[i].Data(), rows * (rows + 1) / 2 * sizeof(Real), cudaMemcpyHostToDevice));
		}
		t.end();
		KALDI_LOG << "copy SpMatrix data to CuNumSpMatrix time: " << t.used_time() << "ms";
	}
}

template <typename Real>
void CuNumPackedMatrix<Real>::CopyFromMats(const std::vector<CuSpMatrix<Real> > &mats)
{
	if (CuDevice::Instantiate().Enabled())
	{
		KALDI_ASSERT(mats.size() == this->nums_ && mats[0].NumRows() == this->rows_);
		if (mats.size() == 0 || mats[0].NumRows() == 0) return;
		int32_cuda sizes = mats.size();
		int32_cuda rows = mats[0].NumRows();
		my_time t;
		t.start();
		KALDI_LOG << "start copy data of CuSpMatrix";
		for (int32_cuda i = 0; i < sizes; i++)
		{
			CU_SAFE_CALL(cudaMemcpy(this->data_ + i * rows * (rows + 1) / 2, mats[i].Data(), rows * (rows + 1) / 2 * sizeof(Real), cudaMemcpyDeviceToDevice));
		}
		t.end();
		KALDI_LOG << "copy CuSpMatrix data to CuNumSpMatrix time: " << t.used_time() << "ms";
	}
}

template <typename Real>
void CuNumPackedMatrix<Real>::CopyFromMat(const CuNumPackedMatrix<Real> &mat)
{
	if (CuDevice::Instantiate().Enabled())
	{
		KALDI_ASSERT(mat.NumSizes() == this->nums_ && mat.NumRows() == this->rows_);
		CU_SAFE_CALL(cudaMemcpy(this->data_, mat.Data(), mat.DataNumBytes(), cudaMemcpyDeviceToDevice));
	}
}

template <typename Real>
void CuNumPackedMatrix<Real>::Scale(Real alpha)
{
	if (CuDevice::Instantiate().Enabled())
	{
		KALDI_ASSERT(NumRows() > 0 && NumSizes() > 0 && NumCols() > 0);
		int nr = NumRows();
		int size = NumSizes();
		int dimBlock(CU1DBLOCK);
		int dimGrid(n_blocks(size * nr * (nr + 1) / 2, CU1DBLOCK));
		my_cuda_scale_linear(this->data_, size * nr * (nr + 1) / 2, alpha, dimGrid, dimBlock);
	}
}

template <typename Real>
void CuNumPackedMatrix<Real>::ScaleDiagNum(Real alpha)
{
	if (CuDevice::Instantiate().Enabled())
	{
		KALDI_ASSERT(NumRows() > 0 && NumSizes() > 0 && NumCols() > 0);
		dim3 Bl(CU2DBLOCK, CU2DBLOCK);
		dim3 Gr(n_blocks(this->NumSizes(), CU2DBLOCK), n_blocks(this->NumRows(), CU2DBLOCK));
		my_cuda_scale_diag_numsp(this->Data(), this->NumSizes(), this->Dim(), alpha, Gr, Bl);
	}
}

template <typename Real>
void CuNumPackedMatrix<Real>::AddVec3(const CuMatrixBase<Real> &mat, Real alpha)
{
	if (CuDevice::Instantiate().Enabled())
	{
		KALDI_ASSERT(mat.NumRows() == NumSizes());
		// KALDI_LOG << "CuNumPackedMatrix nums: " << this->NumSizes() << "    rows: " << this->NumRows() << "    d.stride: " << this->Dim().stride << "    d.rows: " << this->Dim().rows << "    d.cols: " << this->Dim().cols;
		dim3 dimBlock(8, 8, 8);
		dim3 dimGrid(n_blocks(this->NumSizes(), 8), n_blocks(this->NumRows(), 8), n_blocks(this->NumRows(), 8));
		my_cuda_addvec3(this->data_, NumSizes(), mat.Data(), mat.Dim(), alpha, dimGrid, dimBlock);
	}
}


template class CuLinearMatrixBase<float>;
template class CuLinearMatrixBase<double>;
template class CuLinearMatrix<float>;
template class CuLinearMatrix<double>;
template class CuNumMatrixBase<float>;
template class CuNumMatrixBase<double>;
template class CuNumMatrix<float>;
template class CuNumMatrix<double>;
template class CuNumPackedMatrix<float>;
template class CuNumPackedMatrix<double>;
template class CuNumSpMatrix<float>;
template class CuNumSpMatrix<double>;


#endif
