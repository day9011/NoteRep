all:

include ../kaldi.mk


LIBVADDIR =  /home/hanyu/Documents/vad
VADINCLUDEDIR = $(LIBVADDIR)/include
VADINCLUDEDIRFLAGS = $(addprefix -I,$(VADINCLUDEDIR))
VADRUNLIBDIR = -Wl,-rpath=$(LIBVADDIR)/lib
VADLIBDIR = $(LIBVADDIR)/lib
VADLIBDIRFLAGS = $(addprefix -L, $(VADLIBDIR))
VADLIBS = fvad
VADLIBSFLAGS = $(addprefix -l, $(VADLIBS))
CXXFLAGS += -Wpedantic -DHAVE_CONFIG_H $(VADINCLUDEDIRFLAGS) -I. 


#TESTFILES = sre
TESTFILES =

#OBJFILES = sre.o
OBJFILES = fvadfunc.o cpu-init.o ubm.o my-init-extractor.o pcm-reader.o my-ivector-extractor.o \
		   pcm-data.o wav-data.o score.o sre.o cpu-compute.o
#OBJFILES = my-init-extractor.o pcm-reader.o

#fvadfunc.o: %.o: %.cc
#	g++ -g -Wall -O2 -std=c++11 -fPIC -Wpedantic -DHAVE_CONFIG_H $(VADINCLUDEDIRFLAGS) -c $< -o $@

LDLIBS += $(VADRUNLIBDIR) $(VADLIBDIRFLAGS) $(VADLIBSFLAGS)

LIBNAME = mylib

ADDLIBS += ../feat/kaldi-feat.a ../ivector/kaldi-ivector.a ../base/kaldi-base.a ../matrix/kaldi-matrix.a ../gmm/kaldi-gmm.a \
		  ../tree/kaldi-tree.a ../util/kaldi-util.a ../hmm/kaldi-hmm.a ../thread/kaldi-thread.a ../transform/kaldi-transform.a

include ../makefiles/default_rules.mk

