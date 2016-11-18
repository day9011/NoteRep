JAVAFILES = CudaInitUBM SRECompute
CLASSFILE = CudaInitUBM.class SRECompute.class

JARFILE = javacpp.jar

all: build classbuild

build: $(JAVAFILES)

classbuild: $(CLASSFILE)

$(JAVAFILES): $(CLASSFILE)
	java -jar $(JARFILE) $@

%.class: %.java
	javac -cp $(JARFILE) $<

clean:
	rm -f *.class *.so
	rm -rf `find . -type d| grep -E "\.\S+"`
