all:

OPENFST_CXXFLAGS = 
OPENFST_LDLIBS =

EXTRA_CXXFLAGS = -Wno-sign-compare
#EXTRA_CXXFLAGS = -Wno-sign-compare -DGSELECT_TEST
include ../kaldi.mk

AMRWBROOT = /home/gzdinghanyu/amr
LIBVADDIR =  /home/gzdinghanyu/vad
MYINCLUDEDIR = $(LIBVADDIR)/include $(AMRWBROOT)/include/opencore-amrwb
MYINCLUDEDIRFLAGS = $(addprefix -I,$(MYINCLUDEDIR))
MYRUNLIBDIR = -Wl,-rpath=$(LIBVADDIR)/lib -Wl,-rpath=$(AMRWBROOT)/lib
MYLIBDIR = $(LIBVADDIR)/lib $(AMRWBROOT)/lib
MYLIBDIRFLAGS = $(addprefix -L, $(MYLIBDIR))
MYLIBS = fvad opencore-amrwb
MYLIBSFLAGS = $(addprefix -l, $(MYLIBS))
CXXFLAGS += -Wpedantic -DHAVE_CONFIG_H $(MYINCLUDEDIRFLAGS) -I.

LDLIBS += $(MYRUNLIBDIR) $(MYLIBDIRFLAGS) $(MYLIBSFLAGS)


BINFILES = wav_ivector

OBJFILES =

TESTFILES =

ADDLIBS = ../mylib/mylib.a ../cudamatrix/kaldi-cudamatrix.a ../feat/kaldi-feat.a \
		  ../ivector/kaldi-ivector.a ../transform/kaldi-transform.a ../hmm/kaldi-hmm.a \
		  ../gmm/kaldi-gmm.a ../matrix/kaldi-matrix.a ../tree/kaldi-tree.a \
		  ../util/kaldi-util.a ../base/kaldi-base.a


include ../makefiles/default_rules.mk

