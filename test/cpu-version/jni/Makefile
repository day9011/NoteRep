JAVAFILES = InitSRE CpuCompute ScoreDotProduct
PACAGEPATH = /home/hanyu/kaldi-trunk/src/mycode/jni/com/pingan/jni/
TEST = test

JARFILE = javacpp.jar

all: package build test

package:
	make -C $(PACAGEPATH)

build: $(JAVAFILES)

$(JAVAFILES):
	java -jar $(JARFILE) com.pingan.jni.$@

test: $(TEST)

$(TEST):
	javac -cp $(JARFILE):$(PACAGEPATH):. $@.java
	java -cp $(JARFILE):$(PACAGEPATH):. $@


clean:
	rm -f `find . | grep -E "(\.class|\.so)"`
#	rm -rf `find . -type d| grep -E "\.\S+"`
