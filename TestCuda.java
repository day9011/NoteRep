import org.bytedeco.javacpp.*;
import org.bytedeco.javacpp.annotation.*;
import java.io.*;
import java.text.*;

@Properties(value = {
	@Platform(define={"HAVE_POSIX_MEMALIGN", "KALDI_DOUBLEPRECISION 0", "HAVE_EXECINFO_H 1", "HAVE_CXXABI_H 1", "HAVE_ATLAS 1", "HAVE_OPENFST_GE_10400 1", "HAVE_CUDA 1"},
			  include={"cudalib/my-cuda-init.h", "cudalib/my-cuda-compute.h"},
			  includepath("/home/hanyu/kaldi-trunk/src", "/home/hanyu/kaldi-trunk/tools/ATLAS/include", "/home/hanyu/kaldi-trunk/tools/openfst/include", "/home/hanyu/kaldi-trunk/tools/openfst/lib",  "/usr/local/cuda/include"},
			  linkpath={"/home/hanyu/kaldi-trunk/src/lib", "/usr/local/cuda/lib64", "/home/hanyu/kaldi-trunk/tools/openfst/lib", "/usr/lib"},
			  link={"mylib", "cudalib", "kaldi-feat", "kaldi-ivector", "kaldi-transform", "kaldi-base", "kaldi-gmm", "kaldi-tree", "kaldi-cudamatrix", "kaldi-hmm", "kaldi-matrix", "kaldi-thread", "kaldi-util", "m", "dl", "fst", "cublas", "cudart", "cufft"}
			 )
})
@Namespace("kaldi")
public class TestCuda
{
	public static class CudaInitUBM extends Pointer
	{
		static { Loader.load(); }
		public CudaInitUBM() { allocate(); }
		private native void allocate();
		public native @Cast("bool") boolean CudaInitReadFile(@StdString String final_ubm, @StdString String final_ie, @StdString String gmm_ubm, int gpuid);
	}

	public static class SRECompute extends Pointer
	{
		static { Loader.load(); }
		public SRECompute() { allocate(); }
		private native void allocate();
		public native void cuda_sre_compute(CudaInitUBM ubm, @StdString String voice_file, int valid_frames);
	}

	public static void main(String args[])
	{
		CudaInitUBM ubm = new CudaInitUBM();
		SRECompute comp = new SRECompute();
		String voice_file = "/home/hanyu/wavfile/pcm.wav";
		String final_ubm = "/home/hanyu/kaldi-trunk/src/mycode/ubm/final.ubm";
		String final_ie = "/home/hanyu/kaldi-trunk/src/mycode/ubm/final.ie";
		String gmm_ubm = "/home/hanyu/kaldi-trunk/src/mycode/ubm/gmm.ubm";
		int valid_frames = 1000;
		int gpuid = 4;
		ubm.CudaInitReadFile(final_ubm, final_ie, gmm_ubm, gpuid);
		comp.cuda_sre_compute(ubm, voice_file, valid_frames);
		System.out.print("for a test");
	}

}
