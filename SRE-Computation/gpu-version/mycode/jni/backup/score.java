
public class score {
public static void main(String[] args)
{
	ScoreDotProduct score = new ScoreDotProduct();
	String file1 = "/home/hanyu/ivector.txt";
	String file2 = "/home/hanyu/ivector.txt";
	float point = score.score_dot_product(file1, file2);
	System.out.print("for a test");
	System.out.print(point);
}
}
