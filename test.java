
public class test {
public static void main(String[] args)
{
	CudaInitUBM ubm = new CudaInitUBM();
	SRECompute comp = new SRECompute();
	String voice_file = "/home/hanyu/wavfile/pcm.wav";
	String final_ubm = "/home/hanyu/tool400/final.ubm";
	String final_ie = "/home/hanyu/tool400/final-inited.ie";
	String gmm_ubm = "/home/hanyu/tool400/gmm.ubm";
	String ivector_path = "/home/hanyu/ivector.txt";
	int valid_frames = 1000;
	int gpuid = 4;
	ubm.CudaInitReadFile(final_ubm, final_ie, gmm_ubm, gpuid);
	comp.cuda_sre_compute(ubm, voice_file, ivector_path, valid_frames);
	System.out.print("for a test");
}
}
