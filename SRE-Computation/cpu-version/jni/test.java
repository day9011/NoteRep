import com.pingan.jni.InitSRE;
import com.pingan.jni.CpuCompute;
import com.pingan.jni.ScoreDotProduct;

public class test {
public static void main(String[] args)
{
	InitSRE sre = new InitSRE();
	CpuCompute comp = new CpuCompute();
	ScoreDotProduct score = new ScoreDotProduct();
	String voice_file = "/home/hanyu/wavfile/pcm.wav";
	String final_ubm = "/home/hanyu/tool400/final.ubm";
	String final_ie = "/home/hanyu/tool400/final.ie";
	String gmm_ubm = "/home/hanyu/tool400/gmm.ubm";
	String ivector_path = "/home/hanyu/ivector.txt";
	int valid_frames = 1000;
	sre.ReadUBMFile(final_ubm, final_ie, gmm_ubm);
	comp.Compute(sre, voice_file, ivector_path, valid_frames);
	System.out.print("for a test");
}
}
