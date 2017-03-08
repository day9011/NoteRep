all:

OPENFST_CXXFLAGS = 
OPENFST_LDLIBS =

EXTRA_CXXFLAGS = -Wno-sign-compare
#EXTRA_CXXFLAGS = -Wno-sign-compare -DGSELECT_TEST
include ../kaldi.mk

LDFLAGS += $(CUDA_LDFLAGS)
LDLIBS += $(CUDA_LDLIBS)

#BINFILES = score_dot_product score_plda1 score_plda2 score_plda3 \
		   wav_ivector pcm_ivector product_float ie_init cuda cuda_test \
		   my_cuda test_fft

#BINFILES = wav_ivector pcm_ivector cuda cuda_test my_cuda test_fft
#BINFILES = wav_ivector pcm_ivector cuda_test
BINFILES = test_vector pcm_ivector

#OBJFILES = sre.o
OBJFILES =

TESTFILES =

%.o : %.cu
	$(CUDATKDIR)/bin/nvcc -c $< -o $@ $(CUDA_INCLUDE) $(CUDA_FLAGS) $(CUDA_ARCH) -I../

ADDLIBS = ../cudalib/cudalib.a ../mylib/mylib.a ../cudamatrix/kaldi-cudamatrix.a ../feat/kaldi-feat.a \
		  ../ivector/kaldi-ivector.a ../transform/kaldi-transform.a ../hmm/kaldi-hmm.a \
		  ../gmm/kaldi-gmm.a ../matrix/kaldi-matrix.a ../tree/kaldi-tree.a ../thread/kaldi-thread.a \
		  ../util/kaldi-util.a ../base/kaldi-base.a

#ADDLIBS = ../cudalib/cudalib.a ../mylib/mylib.a  ../cudamatrix/kaldi-cudamatrix.a ../feat/kaldi-feat.a \
		  ../ivector/kaldi-ivector.a ../transform/kaldi-transform.a ../hmm/kaldi-hmm.a \
		  ../gmm/kaldi-gmm.a ../tree/kaldi-tree.a ../thread/kaldi-thread.a ../matrix/kaldi-matrix.a \
		  ../util/kaldi-util.a ../base/kaldi-base.a
#ADDLIBS = ../feat/kaldi-feat.a ../ivector/kaldi-ivector.a ../hmm/kaldi-hmm.a ../gmm/kaldi-gmm.a \
    	../tree/kaldi-tree.a ../thread/kaldi-thread.a ../matrix/kaldi-matrix.a \
    	../util/kaldi-util.a ../base/kaldi-base.a ../mylib/mylib.a \
		../transform/kaldi-transform.a ../decoder/kaldi-decoder.a ../lat/kaldi-lat.a

include ../makefiles/default_rules.mk

