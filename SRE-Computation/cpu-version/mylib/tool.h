#ifndef MY_TOOL
#define MY_TOOL

#include "matrix/matrix-lib.h"
#include <stdio.h>
#include <stdlib.h>
#include "util/kaldi-io.h"
#include "mylib/gettime.h"

template <class T>
void out_mat_to_file(const T &mat, std::string filename)
{
	Output out_(filename, false, false);
	if (out_.IsOpen())
	{
		for (int i = 0; i < mat.NumRows(); i++)
		{
			out_.Stream() << "[";
			for (int j = 0; j < mat.NumCols(); j++)
				out_.Stream() << " " << mat(i, j);
			out_.Stream() << " ]\n";
		}
	}
}

template <class T>
void out_mat_to_file(T *mat, std::string filename)
{
	Output out_(filename, false, false);
	if (out_.IsOpen())
	{
		for (int i = 0; i < mat->NumRows(); i++)
		{
			out_.Stream() << "[";
			for (int j = 0; j < mat->NumCols(); j++)
				out_.Stream() << " " << (*mat)(i, j);
			out_.Stream() << " ]\n";
		}
	}
}

template <class T>
bool out_vec_to_file(const T &vec, std::string filename)
{
	Output out_(filename, false, false);
	if (out_.IsOpen())
	{
		out_.Stream() << "[";
		for (int i = 0; i < vec.Dim(); i++)
				out_.Stream() << " " << vec(i);
		out_.Stream() << " ]\n";
		return true;
	}
	else return false;
	return true;
}

template <class T>
bool out_vec_to_file(const T *vec, std::string filename)
{
	Output out_(filename, false, false);
	if (out_.IsOpen())
	{
		out_.Stream() << "[";
		for (int i = 0; i < vec->Dim(); i++)
				out_.Stream() << " " << (*vec)(i);
		out_.Stream() << " ]\n";
		return true;
	}
	else return false;
	return true;
}

#endif
