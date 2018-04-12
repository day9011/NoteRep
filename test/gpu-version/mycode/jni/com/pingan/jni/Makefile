CLASSFILES := CudaInitUBM.class SRECompute.class ScoreDotProduct.class

JARFILE = /home/hanyu/cudaLib/src/mycode/jni/javacpp.jar

all: classbuild

classbuild: $(CLASSFILES)

%.class: %.java
	javac -cp $(JARFILE) $<

clean:
	rm -f *.class *.so
#	rm -rf `find . -type d| grep -E "\.\S+"`
