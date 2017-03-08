command example:
=============================================
pcm_ivector --config=./compute.conf /home/hanyu/kaldi-trunk/src/mycode/10.wav final.ubm final.ie gmm.ubm valid_frames<br>
score_dot_product 1.txt 2.txt<br>
score_plda3 1.txt 2.txt plda transform.mat<br>
wav_ivector调用与pcm_ivector一样

编译流程：
=============================================
1.src/Makefile,修改SUBDIRS，MEMTESTDIRS，Dependency list，添加自己的目录名称<br>
2.在依赖列表中添加自己代码的依赖目录，自己目录下的.depend.mk为自己代码中间文件的依赖文件<br>
3.src/mycode，src/mylib，复制一份命令目录中的Makefile文件，修改BINFILES为自己名称，OBJFILES，为依赖的.o文件，添加ADDLIBS，为依赖代码目录下的.a文件，库添加的顺序一定要正确，不然报引用错误<br>
4.执行./configure（--shared 动态依赖库模式，这里不选，用--static模式）<br>
5.执行make depend构建依赖<br>
6.在src目录下执行命令make<br>
如果只有自己代码有改动，没有依赖变化，可以在自己代码目录make

注意事项：
=============================================
1.提取ivector前先使用fgmm-global-to-gmm命令将fgmm转换成gmm模型<br>
2.使用ie_init工具将final.ie初始化计算<br>
3.在matrix/kaldi-vector.h中改变CopyFromPtr方法的权限为public，方便之后调用<br>
4.在gmm/full-gmm.cc文件中的ComputeGconsts函数中，只对gconsts进行初始化，不做其他操作<br>
5.当遇到权限问题（一般是Data获取的方法）时，只需要将该类方法的权限改为public <br>
6.gpu版本需要将cudamatrix中的cu-device替换到kaldi相应目录下的cu-device文件 <br>
7.gpu版本在src/makefiles目录下的linux_cuda.mk, linux_x86_64_cuda.mk文件中添加LDLIBS += -lcublas -lcudart -lcufft -lcurand，作用是添加cuda的第三方库

现在目录结构为，version为版本库，tool为工具库，里面存放的为测试工具，评分代码以及算法最终的流程代码。
.bak文件为老版本，没有对其继续更新

内存挂载:
=============================================
1.在linux下有文件目录是直接使用的内存空间，空间默认为物理内存的一半<br>
    Redhat是/dev/shm，ubuntu为/run/shm，可以使用df -h查看<br>
2.修改占用内存大小，修改/etc/fstab中tmpfs   /run/shm	tmpfs,defaults,size=512m	0 0 <br>
	再执行mount -o remount /run/shm <br>
3.mkdir /run/shm/tool <br>
4.mkdir /tool <br>
5.chmod 1777 /dev/shm/tool <br>
6.mount --bind /run/shm/tool /tool <br>
强调下：tmpfs 数据在重新启动之后不会保留，重启tmpfs 数据会丢失，所以有必要做一些脚本做诸如加载，绑定的操作！ <br>
