#ifndef MY_CUDA_TOOL
#define MY_CUDA_TOOL

#include "cudalib/my-cuda-data-struct.h"
#include <stdio.h>
#include <stdlib.h>
#include "util/kaldi-io.h"
#include "mylib/gettime.h"
#include "mylib/tool.h"
#include "cudamatrix/cu-vector.h"
#include "cudamatrix/cu-common.h"


template <typename Real>
bool out_num_sp_matrix_to_file(const CuNumPackedMatrix<Real> &nmat, std::string filename)
{
	Output out_(filename, false, false);
	if (out_.IsOpen())
	{
		Real *data;
		data = (Real*)malloc(nmat.DataNumBytes());
		CU_SAFE_CALL(cudaMemcpy(data, nmat.Data(), nmat.DataNumBytes(), cudaMemcpyDeviceToHost));
		MatrixDim d = nmat.Dim();
		KALDI_LOG << "nmat stride:" << d.stride << "    rows:" << d.rows;
		for (int n = 0; n < nmat.NumSizes(); n++)
		{
			out_.Stream() << "[";
			for (int i = 0; i < d.stride; i++)
				out_.Stream() << " " << *(data + n * d.stride + i);
			out_.Stream() << " ]\n";
		}
		free(data);
	}
	else return false;
	return true;
}

inline bool cuda_ivector_normalize_length(CuVector<double> &ivector)
{
    double norm = ivector.Norm(2.0);
    double ratio = norm / sqrt(ivector.Dim());
    if (ratio == 0.0)
    {
        std::cout << "Zero iVector" << std::endl;
        return false;
    }
    else ivector.Scale(1.0 / ratio);
    return true;
}

inline bool cuda_ivector_mean(CuVector<double> &ivector)
{
    if (ivector.Dim() < 1)
    {
        std::cout << "empty ivector" << std::endl;
        return false;
    }
    ivector.Scale(1.0 / 1);
    return true;
}


#endif
