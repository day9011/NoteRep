#ifndef MY_CUDA_DATA_STRUCT
#define MY_CUDA_DATA_STRUCT

#if HAVE_CUDA == 1
#include "base/kaldi-common.h"
#include "cudamatrix/cu-common.h"
#include "cudamatrix/cu-matrixdim.h"
#include "cudamatrix/cu-value.h"
#include "cudamatrix/cu-matrix-lib.h"
#include "cudamatrix/cu-matrixdim.h"
#include "matrix/matrix-lib.h"
#include "util/kaldi-io.h"
#include "mylib/gettime.h"
#include "cudalib/my-cuda-function-kernel.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>

using namespace kaldi;

class MatrixIntBase;
class MatrixInt;
class CuMatrixIntBase;
class CuMatrixInt;

class MatrixIntBase
{
	public:
		friend class CuMatrixIntBase;
		friend class MatrixInt;
		friend class CuMatrixInt;
		inline int32_cuda NumRows() const {return rows_;}
		inline int32_cuda NumCols() const {return cols_;}
		inline int32_cuda* Data() const {return data_;}

		void CopyFromMat(const MatrixIntBase &mat);
		void CopyFromMat(const CuMatrixIntBase &mat);

		inline int32_cuda operator() (int32_cuda row, int32_cuda col) const
		{
			KALDI_ASSERT(row < rows_ && col < cols_ );
			return *(data_ + row * cols_ + col);
		}

	protected:
		MatrixIntBase() : data_(NULL), rows_(0), cols_(0) {}
		~MatrixIntBase() {}
		int32_cuda *data_;
		int32_cuda rows_;
		int32_cuda cols_;
	private:
		KALDI_DISALLOW_COPY_AND_ASSIGN(MatrixIntBase);

};

class MatrixInt: public MatrixIntBase
{
	public:
		MatrixInt() {}

		MatrixInt(int32_cuda rows, int32_cuda cols)
		{
			Resize(rows, cols);
		}

		MatrixInt(const MatrixIntBase &other)
		{
			this->Resize(other.NumRows(), other.NumCols());
			this->CopyFromMat(other);
		}

		MatrixInt &operator = (const MatrixIntBase &other)
		{
			this->Resize(other.NumRows(), other.NumCols());
			this->CopyFromMat(other);
			return *this;
		}

		MatrixInt &operator = (const MatrixInt &other)
		{
			this->Resize(other.NumRows(), other.NumCols());
			this->CopyFromMat(other);
			return *this;
		}


		void Resize(int32_cuda rows, int32_cuda cols);
		~MatrixInt(){ Destroy(); }
	private:
		void Destroy();
};

class CuMatrixIntBase
{
	public:
		friend class MatrixInt;
		friend class MatrixIntBase;
		friend class CuMatrixInt;

		inline int32_cuda NumRows() const { return rows_; }
		inline int32_cuda NumCols() const { return cols_; }
		inline int32_cuda* Data() const { return data_; }

		void CopyFromMat(const MatrixIntBase &mat);
		void CopyFromMat(const CuMatrixIntBase &mat);

		inline int32_cuda operator() (int32_cuda row, int32_cuda col) const
		{
			KALDI_ASSERT(row < rows_ && col < cols_);
			return CuValue<int32_cuda>(data_ + row * cols_ + col);
		}

		inline CuValue<int32_cuda> operator() (int32_cuda row, int32_cuda col)
		{
			KALDI_ASSERT(row < rows_ && col < cols_);
			return CuValue<int32_cuda>(data_ + row * cols_ + col);
		}

	protected:
		CuMatrixIntBase() : data_(NULL), rows_(0), cols_(0) {}
		~CuMatrixIntBase() {}
		int32_cuda *data_;
		int32_cuda rows_;
		int32_cuda cols_;
	private:
		KALDI_DISALLOW_COPY_AND_ASSIGN(CuMatrixIntBase);
};

class CuMatrixInt : public CuMatrixIntBase
{
	public:
		CuMatrixInt() {}
		CuMatrixInt(int32_cuda rows, int32_cuda cols)
		{
			Resize(rows, cols);
		}

		CuMatrixInt(const CuMatrixIntBase &other)
		{
			this->Resize(other.NumRows(), other.NumCols());
			this->CopyFromMat(other);
		}

		CuMatrixInt &operator = (const CuMatrixInt &other)
		{
			this->Resize(other.NumRows(), other.NumCols());
			this->CopyFromMat(other);
			return *this;
		}

		CuMatrixInt &operator = (const CuMatrixIntBase &other)
		{
			this->Resize(other.NumRows(), other.NumCols());
			this->CopyFromMat(other);
			return *this;
		}


		void Resize(int32_cuda rows, int32_cuda cols);
		~CuMatrixInt(){ Destroy(); }
	private:
		void Destroy();
};

inline void printMatInt(MatrixInt const mat)
{
	for (int i = 0; i < mat.NumRows(); i++)
	{
		std::cout << "[";
		for (int j = 0; j < mat.NumCols(); j++)
			std::cout << " " << mat(i, j);
		std::cout << " ]\n";
	}
}

#if HAVE_CUDA == 1
//a array matrix
template<typename Real> class CuNumMatrix;
template<typename Real> class CuNumMatrixBase;
template<typename Real> class CuNumPackedMatrix;
template<typename Real> class CuNumSpMatrix;
template<typename Real> class CuLinearMatrixBase;
template<typename Real> class CuLinearMatrix;

template <typename Real>
class CuLinearMatrixBase
{
	public:
		friend CuLinearMatrix<Real>;
		inline int32_cuda NumRows() const { return rows_; }
		inline int32_cuda NumCols() const { return cols_; }
		inline const Real* Data() const { return data_; }
		inline Real* Data() { return data_; }

		void SetZero();

		void CopyFromMat(const Matrix<Real> &mats);
		void CopyFromMat(const CuMatrix<Real> &mats);
		void CopyFromMat(const CuLinearMatrixBase<Real> &mat);

		//void GenerateSuperMatrix();

		size_t DataNumBytes()
		{
			size_t num_bytes = static_cast<size_t>(rows_ * cols_ * sizeof(Real));
			return num_bytes;
		}

		size_t DataNumBytes() const
		{
			size_t num_bytes = static_cast<size_t>(rows_ * cols_ * sizeof(Real));
			return num_bytes;
		}


		inline Real operator() (int32_cuda row, int32_cuda col) const
		{
			KALDI_ASSERT(row < rows_ && col < cols_);
			return CuValue<Real>(data_ + row * cols_ + col);
		}

		inline CuValue<Real> operator() (int32_cuda row, int32_cuda col)
		{
			KALDI_ASSERT(row < rows_ && col < cols_);
			return CuValue<Real>(data_ + row * cols_ + col);
		}
	protected:
		CuLinearMatrixBase(): data_(NULL), rows_(0), cols_(0) {}
		~CuLinearMatrixBase() {}
		Real *data_;
		int32_cuda rows_;
		int32_cuda cols_;
	private:
		KALDI_DISALLOW_COPY_AND_ASSIGN(CuLinearMatrixBase);
};

template <typename Real>
class CuLinearMatrix: public CuLinearMatrixBase<Real>
{
	public:
		CuLinearMatrix() {}
		CuLinearMatrix(int32_cuda rows, int32_cuda cols)
		{
			Resize(rows, cols);
		}

		CuLinearMatrix(const CuLinearMatrixBase<Real> &other)
		{
			this->Resize(other.NumRows(), other.NumCols());
			this->CopyFromMat(other);
		}

		CuLinearMatrix<Real> &operator = (const CuLinearMatrix<Real> &other)
		{
			this->Resize(other.NumRows(), other.NumCols());
			this->CopyFromMat(other);
			return *this;
		}

		CuLinearMatrix<Real> &operator = (const CuLinearMatrixBase<Real> &other)
		{
			this->Resize(other.NumRows(), other.NumCols());
			this->CopyFromMat(other);
			return *this;
		}

		~CuLinearMatrix() { Destroy(); }

		void Resize(int32_cuda rows, int32_cuda cols);
	private:
		void Destroy();
};



template <typename Real>
class CuNumMatrixBase
{
	public:
		friend CuNumMatrix<Real>;
		inline int32_cuda NumSizes() const { return nums_; }
		inline int32_cuda NumRows() const { return rows_; }
		inline int32_cuda NumCols() const { return cols_; }
		inline const Real* Data() const { return data_; }
		inline Real* Data() { return data_; }

		void SetZero();

		void CopyFromMats(const std::vector<Matrix<Real> > &mats);
		void CopyFromMats(const std::vector<CuMatrix<Real> > &mats);
		void CopyFromMat(const CuNumMatrixBase<Real> &mat);

		void Scale(Real alpha);

		//void GenerateSuperMatrix();

		size_t DataNumBytes()
		{
			size_t num_bytes = static_cast<size_t>(nums_ * rows_ * cols_ * sizeof(Real));
			return num_bytes;
		}

		size_t DataNumBytes() const
		{
			size_t num_bytes = static_cast<size_t>(nums_ * rows_ * cols_ * sizeof(Real));
			return num_bytes;
		}


		inline Real operator() (int32_cuda seq, int32_cuda row, int32_cuda col) const
		{
			KALDI_ASSERT(seq < nums_ && row < rows_ && col < cols_);
			return CuValue<Real>(data_ + seq * rows_ * cols_ + row * cols_ + col);
		}

		inline CuValue<Real> operator() (int32_cuda seq, int32_cuda row, int32_cuda col)
		{
			KALDI_ASSERT(seq < nums_ && row < rows_ && col < cols_);
			return CuValue<Real>(data_ + seq * rows_ * cols_ + row * cols_ + col);
		}
	protected:
		CuNumMatrixBase(): data_(NULL), nums_(0), rows_(0), cols_(0) {}
		~CuNumMatrixBase() {}
		Real *data_;
		//Real *super_data_; // super matrix data
		int32_cuda nums_;
		int32_cuda rows_;
		int32_cuda cols_;
	private:
		KALDI_DISALLOW_COPY_AND_ASSIGN(CuNumMatrixBase);
};

template <typename Real>
class CuNumMatrix: public CuNumMatrixBase<Real>
{
	public:
		CuNumMatrix() {}
		CuNumMatrix(int32_cuda nums, int32_cuda rows, int32_cuda cols)
		{
			Resize(nums, rows, cols);
		}

		CuNumMatrix(const CuNumMatrix<Real> &other)
		{
			this->Resize(other.NumSizes(), other.NumRows(), other.NumCols());
			this->CopyFromMat(other);
		}

		CuNumMatrix<Real> &operator = (const CuNumMatrix<Real> &other)
		{
			this->Resize(other.NumSizes(), other.NumRows(), other.NumCols());
			this->CopyFromMat(other);
			return *this;
		}

		CuNumMatrix<Real> &operator = (const CuNumMatrixBase<Real> &other)
		{
			this->Resize(other.NumSizes(), other.NumRows(), other.NumCols());
			this->CopyFromMat(other);
			return *this;
		}

		~CuNumMatrix() { Destroy(); }

		void Resize(int32_cuda nums, int32_cuda rows, int32_cuda cols);
	private:
		void Destroy();
};


template <typename Real>
class CuNumPackedMatrix
{
	public:
		friend class CuNumSpMatrix<Real>;

		inline int32_cuda NumSizes() const { return nums_; }
		inline int32_cuda NumRows() const { return rows_; }
		inline int32_cuda NumCols() const { return rows_; }
		
		//Here, d_.stride is the dim of spmatrix dim and d_.rows is the number of matrix of CuNumSpMatrix
		inline MatrixDim Dim() const { return d_; }

		inline const Real* Data() const { return data_; }
		Real* Data() { return data_; }
		inline CuValue<Real> operator() (int32_cuda seq, int32_cuda row, int32_cuda col)
		{
			KALDI_ASSERT(seq < nums_ && row < rows_ && col < rows_);
			if (row < col)
				std::swap(row, col);
			return CuValue<Real>(data_ + seq * rows_ * (rows_ + 1) / 2 + row * (row + 1) / 2 + col);
		}

		inline Real operator() (int32_cuda seq, int32_cuda row, int32_cuda col) const
		{
			KALDI_ASSERT(seq < nums_ && row < rows_ && col < rows_);
			if (row < col)
				std::swap(row, col);
			return CuValue<Real>(data_ + seq * rows_ * (rows_ + 1) / 2 + row * (row + 1) / 2 + col);
		}

		void CopyFromMats(const std::vector<SpMatrix<Real> > &mats);
		void CopyFromMats(const std::vector<CuSpMatrix<Real> > &mats);
		void CopyFromMat(const CuNumPackedMatrix<Real> &mat);

		void AddVec3(const CuMatrixBase<Real> &mat, Real alpha);

		void Scale(Real alpha);
		void ScaleDiagNum(Real alpha);

		size_t DataNumBytes()
		{
			size_t nr = static_cast<size_t>(rows_);
			size_t num_bytes = static_cast<size_t>(nums_) * (nr * (nr + 1) / 2) * sizeof(Real);
			return num_bytes;
		}

		size_t DataNumBytes() const
		{
			size_t nr = static_cast<size_t>(rows_);
			size_t num_bytes = static_cast<size_t>(nums_) * (nr * (nr + 1) / 2) * sizeof(Real);
			return num_bytes;
		}

		void SetZero();

	protected:
		CuNumPackedMatrix(): data_(NULL), nums_(0), rows_(0) {}
		~CuNumPackedMatrix() {}
		Real *data_;
		int32_cuda nums_;
		int32_cuda rows_;
		//MatrixDim in this data struct is defferent from others, stride is count of elements of one matrix, rows is count of matrixs, cols is count of rows of one matrix
		MatrixDim d_;
	private:
		KALDI_DISALLOW_COPY_AND_ASSIGN(CuNumPackedMatrix);
};

template <typename Real>
class CuNumSpMatrix: public CuNumPackedMatrix<Real>
{
	public:
		CuNumSpMatrix() {}
		
		CuNumSpMatrix(const CuNumSpMatrix<Real> &other)
		{
			this->Resize(other.NumSizes(), other.NumRows());
			this->CopyFromMat(other);
		}

		CuNumSpMatrix(int32_cuda nums, int32_cuda rows)
		{
			Resize(nums, rows);
		}

		void Resize(int32_cuda nums, int32_cuda rows);

		CuNumSpMatrix<Real> &operator = (const CuNumSpMatrix<Real> &other)
		{
			this->Resize(other.NumSizes(), other.NumRows());
			this->CopyFromMat(other);
			return *this;
		}

		CuNumSpMatrix<Real> &operator = (const CuNumPackedMatrix<Real> &other)
		{
			this->Resize(other.NumSizes(), other.NumRows());
			this->CopyFromMat(other);
			return *this;
		}

		~CuNumSpMatrix() { Destroy(); }

	private:
		void Destroy();
};


//HAVE_CUDA
#endif

#endif

#endif
