all:

OPENFST_CXXFLAGS = 
OPENFST_LDLIBS =

EXTRA_CXXFLAGS = -Wno-sign-compare
#EXTRA_CXXFLAGS = -Wno-sign-compare -DGSELECT_TEST
include ../kaldi.mk

AMRWBINCLUDE = /home/hanyu/Documents/amr/include/opencore-amrwb
AMRWBLIB = /home/hanyu/Documents/amr/lib

CXXFLAGS += -I$(AMRWBINCLUDE)
LDLIBS += -Wl,-rpath,$(AMRWBLIB) -lopencore-amrwb


#BINFILES = score_dot_product score_plda1 score_plda2 score_plda3 \
		   wav_ivector pcm_ivector product_float ie_init cuda cuda_test \
		   my_cuda test_fft

BINFILES = pcm_ivector wav_ivector vad_test score_dot_product score_plda_norm

OBJFILES =

TESTFILES =

ADDLIBS = ../mylib/mylib.a ../cudamatrix/kaldi-cudamatrix.a ../feat/kaldi-feat.a \
		  ../ivector/kaldi-ivector.a ../transform/kaldi-transform.a ../hmm/kaldi-hmm.a \
		  ../gmm/kaldi-gmm.a ../matrix/kaldi-matrix.a ../tree/kaldi-tree.a ../thread/kaldi-thread.a \
		  ../util/kaldi-util.a ../base/kaldi-base.a


include ../makefiles/default_rules.mk

