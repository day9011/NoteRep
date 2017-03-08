all:

include ../kaldi.mk

#TESTFILES = sre
TESTFILES =

#OBJFILES = sre.o
OBJFILES = cpu-init.o ubm.o my-init-extractor.o pcm-reader.o my-ivector-extractor.o \
		   pcm-data.o wav-data.o score.o sre.o cpu-compute.o
#OBJFILES = my-init-extractor.o pcm-reader.o

LIBNAME = mylib

ADDLIBS = ../feat/kaldi-feat.a ../ivector/kaldi-ivector.a ../base/kaldi-base.a ../matrix/kaldi-matrix.a ../gmm/kaldi-gmm.a \
		  ../tree/kaldi-tree.a ../util/kaldi-util.a ../hmm/kaldi-hmm.a ../thread/kaldi-thread.a ../transform/kaldi-transform.a
#ADDLIBS = ../util/kaldi-util.a ../feat/kaldi-feat.a ../ivector/kaldi-ivector.a ../hmm/kaldi-hmm.a \
    	../tree/kaldi-tree.a ../thread/kaldi-thread.a ../matrix/kaldi-matrix.a \
    	../base/kaldi-base.a ../transform/kaldi-transform.a \
		../decoder/kaldi-decoder.a ../lat/kaldi-lat.a ./gmm/kaldi-gmm.a

include ../makefiles/default_rules.mk

