package com.pingan.jni;

import org.bytedeco.javacpp.*;
import org.bytedeco.javacpp.annotation.*;

@Properties(value = {
	@Platform(define={"HAVE_POSIX_MEMALIGN", "KALDI_DOUBLEPRECISION 0", "HAVE_EXECINFO_H 1", "HAVE_CXXABI_H 1", "HAVE_ATLAS 1", "HAVE_OPENFST_GE_10400 1", "HAVE_CUDA 1"},
			  include={"mylib/cpu-init.h", "mylib/cpu-compute.h"},
			  includepath={"/home/hanyu/kaldi-trunk/src", "/home/hanyu/kaldi-trunk/tools/ATLAS/include", "/home/hanyu/kaldi-trunk/tools/openfst/include", "/home/hanyu/kaldi-trunk/tools/openfst/lib",  "/usr/local/cuda/include"},
			  linkpath={"/home/hanyu/kaldi-trunk/src/lib", "/usr/local/cuda/lib64", "/home/hanyu/kaldi-trunk/tools/openfst/lib", "/usr/lib"},
			  link={"mylib", "cudalib", "kaldi-feat", "kaldi-ivector", "kaldi-transform", "kaldi-base", "kaldi-gmm", "kaldi-tree", "kaldi-cudamatrix", "kaldi-hmm", "kaldi-matrix", "kaldi-thread", "kaldi-util", "m", "dl", "fst", "cublas", "cudart", "cufft"}
			 )
})

public class CpuCompute extends Pointer
{
	static { Loader.load(); }
	public CpuCompute() { allocate(); }
	private native void allocate();
	public native @Cast("bool") boolean Compute(InitSRE sre, @StdString String voice_file, @StdString String ivector_path, int valid_frames);
}

