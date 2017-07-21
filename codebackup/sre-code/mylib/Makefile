all:

include ../kaldi.mk


AMRWBROOT = /home/gzdinghanyu/amr
LIBVADDIR =  /home/gzdinghanyu/fvad
MYINCLUDEDIR = $(LIBVADDIR)/include $(AMRWBROOT)/include/opencore-amrwb
MYINCLUDEDIRFLAGS = $(addprefix -I,$(MYINCLUDEDIR))
MYRUNLIBDIR = -Wl,-rpath=$(LIBVADDIR)/lib -Wl,-rpath=$(AMRWBROOT)/lib
MYLIBDIR = $(LIBVADDIR)/lib $(AMRWBROOT)/lib
MYLIBDIRFLAGS = $(addprefix -L, $(MYLIBDIR))
MYLIBS = fvad opencore-amrwb
MYLIBSFLAGS = $(addprefix -l, $(MYLIBS))
CXXFLAGS += -Wpedantic -DHAVE_CONFIG_H $(MYINCLUDEDIRFLAGS) -I.


#TESTFILES = sre
TESTFILES =

#OBJFILES = sre.o
OBJFILES = fvadfunc.o cpu-init.o ubm.o pcm-reader.o \
		   pcm-data.o wav-data.o score.o sre.o cpu-compute.o score-plda.o
#OBJFILES = my-init-extractor.o pcm-reader.o

#fvadfunc.o: %.o: %.cc
#	g++ -g -Wall -O2 -std=c++11 -fPIC -Wpedantic -DHAVE_CONFIG_H $(VADINCLUDEDIRFLAGS) -c $< -o $@

LDLIBS += $(MYRUNLIBDIR) $(MYLIBDIRFLAGS) $(MYLIBSFLAGS)

LIBNAME = mylib

ADDLIBS += ../feat/kaldi-feat.a ../ivector/kaldi-ivector.a ../base/kaldi-base.a ../matrix/kaldi-matrix.a ../gmm/kaldi-gmm.a \
		  ../tree/kaldi-tree.a ../util/kaldi-util.a ../hmm/kaldi-hmm.a ../transform/kaldi-transform.a

include ../makefiles/default_rules.mk

