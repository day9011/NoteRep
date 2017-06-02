此处为代码记录库，存放的是平时项目的代码例子：
=============================================
JNI:
============================================
使用的是javacpp的第三方工具，github为https://github.com/bytedeco/javacpp<br>
	其中改写的文件配置文档地址：http://www.programcreek.com/java-api-examples/index.php?source_dir=javacpp-master/src/main/java/org/bytedeco/javacpp/annotation/Platform.java <br>
	添加C++选项：java -cp /home/hanyu/kaldi-trunk/src/ -jar javacpp.jar  -Xcompiler -pthread -Xcompiler -Wno-sign-compare -Xcompiler -Wno-unused-local-typedefs -Xcompiler -Winit-self<br>
	静态库编译成动态库：gcc -shared -Wl,-sonambmylib.$(VER) -o libmylib.so $(OBJECTS)  -Wl,--whole-archive $(LIBS_TO_LINK) -Wl,--no-whole-archive  $(REGULAR_LIBS)<br>
	一般情况下需要将静态库使用fPIC选项编译<br>

CUDA：
=============================================
.cu文件为cuda代码

cpp2py:
=============================================
此部分代码是使用swig来将C++转换为python接口
