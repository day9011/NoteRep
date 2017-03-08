import org.bytedeco.javacpp.*;
import org.bytedeco.javacpp.annotation.*;

@Properties(value = {
	@Platform(define={"HAVE_POSIX_MEMALIGN", "KALDI_DOUBLEPRECISION 0", "HAVE_EXECINFO_H 1", "HAVE_CXXABI_H 1", "HAVE_ATLAS 1", "HAVE_OPENFST_GE_10400 1", "HAVE_CUDA 1"},
			  include={"cudalib/my-cuda-init.h", "cudalib/my-cuda-compute.h"},
			  includepath={"/home/hanyu/cudaLib/src", "/home/hanyu/cudaLib/tools/ATLAS/include", "/home/hanyu/cudaLib/tools/openfst/include", "/home/hanyu/cudaLib/tools/openfst/lib",  "/usr/local/cuda/include"},
			  linkpath={"/home/hanyu/cudaLib/src/lib", "/usr/local/cuda/lib64", "/home/hanyu/cudaLib/tools/openfst/lib", "/usr/lib"},
			  link={"mylib", "cudalib", "kaldi-feat", "kaldi-ivector", "kaldi-transform", "kaldi-base", "kaldi-gmm", "kaldi-tree", "kaldi-cudamatrix", "kaldi-hmm", "kaldi-matrix", "kaldi-thread", "kaldi-util", "m", "dl", "fst", "cublas", "cudart", "cufft"}
			 )
})

public class SRECompute extends Pointer
{
	static { Loader.load(); }
	public SRECompute() { allocate(); }
	private native void allocate();
	public native @Cast("bool") boolean cuda_sre_compute(CudaInitUBM ubm, @StdString String voice_file, @StdString String ivector_path, int valid_frames);
}

