all:

include ../kaldi.mk

TESTFILES =

OBJFILES =  my-cuda-function-kernel.o my-cuda-data-struct.o my-cuda-mel-computations.o\
			my-cuda-mfcc.o my-cuda-init.o my-cuda-gmm.o my-cuda-fgmm.o \
			my-cuda-ie.o my-cuda-sre.o my-cuda-compute.o 

LIBNAME = cudalib

CUDATKDIR = /usr/local/cuda
LDFLAGS += $(CUDA_LDFLAGS)
LDLIBS += $(CUDA_LDLIBS)

ifeq ($(CUDA), true)
	#Default compute capability architectures we compile with
    CUDA_ARCH=-gencode arch=compute_37,code=sm_37
	#Get the CUDA Toolkit version (remove decimal point char)
    CUDA_VERSION=$(shell $(CUDATKDIR)/bin/nvcc -V | grep release | sed -e 's|.*release ||' -e 's|,.*||' -e 's|\.||')
	#For toolkit 4.2 or newer, add the compute capability 3.0·
    CUDA_VER_GT_4_2 := $(shell [ $(CUDA_VERSION) -ge 42 ] && echo true)
    ifeq ($(CUDA_VER_GT_4_2), true)
        CUDA_ARCH += -gencode arch=compute_30,code=sm_30
    endif
	#For toolkit 5.0 or newer, add the compute capability 3.5·
    CUDA_VER_GT_5_0 := $(shell [ $(CUDA_VERSION) -ge 50 ] && echo true)
    ifeq ($(CUDA_VER_GT_5_0), true)
        CUDA_ARCH += -gencode arch=compute_35,code=sm_35
    endif
	#For toolkit 6.0 or newer, add the compute capability 5.0
    CUDA_VER_GT_6_0 := $(shell [ $(CUDA_VERSION) -ge 60 ] && echo true)
    ifeq ($(CUDA_VER_GT_6_0), true)
        CUDA_ARCH += -gencode arch=compute_50,code=sm_50
    endif
	#For toolkit older than 6.5, add the compute capability 1.0
    CUDA_VER_GT_6_5 := $(shell [ $(CUDA_VERSION) -ge 65 ] && echo true)
    ifneq ($(CUDA_VER_GT_6_5), true)
        CUDA_ARCH += -gencode arch=compute_37,code=sm_37 \
					 -gencode arch=compute_37,code=sm_37·
    endif
endif

%.o : %.cu
	$(CUDATKDIR)/bin/nvcc -c $< -o $@ $(CUDA_INCLUDE) $(CUDA_FLAGS) $(CUDA_ARCH) -I ../

ADDLIBS = ../cudamatrix/kaldi-cudamatrix.a ../feat/kaldi-feat.a ../ivector/kaldi-ivector.a ../gmm/kaldi-gmm.a ../hmm/kaldi-hmm.a \
		  ../mylib/mylib.a ../matrix/kaldi-matrix.a ../util/kaldi-util.a ../base/kaldi-base.a

include ../makefiles/default_rules.mk

