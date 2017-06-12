CXX=g++
SWIG=swig
KALDIROOT= /home/hanyu/Documents/kaldi
LIBVADROOT= /home/hanyu/Documents/vad
INCLUDE= . $(KALDIROOT)/src $(KALDIROOT)/tools/ATLAS/include $(KALDIROOT)/tools/openfst/include /usr/include/python2.7 $(LIBVADROOT)/include
INCLUDEFLAGS= $(addprefix -I,$(INCLUDE))
SWIGFLAGS= -python -c++
MACRO=HAVE_CXXABI_H HAVE_ATLAS KALDI_DOUBLEPRECISION=0 HAVE_EXECINFO_H=1
MACROFLAGS=$(addprefix -D,$(MACRO))
CXXFLAGS= -std=c++11 -Wall -Wno-sign-compare -Wno-unused-local-typedefs -Wno-deprecated-declarations -Winit-self -msse -msse2 -pthread -g  -fPIC
SOFLAGS= -Wl,--no-undefined -Wl,--as-needed  -Wl,--no-whole-archive -rdynamic
RUNLIBS= -Wl,-rpath=$(KALDIROOT)/tools/openfst/lib -Wl,-rpath=$(KALDIROOT)/src/lib -Wl,-rpath=$(LIBVADROOT)/lib
LIBSPATH= $(KALDIROOT)/tools/openfst/lib $(KALDIROOT)/src/lib $(LIBVADROOT)/lib
LIBSFLAGS= $(addprefix -L, $(LIBSPATH))
LDLIB= mylib kaldi-feat kaldi-ivector kaldi-transform kaldi-hmm kaldi-gmm kaldi-matrix kaldi-tree kaldi-thread kaldi-util kaldi-base m pthread dl python2.7 fvad
LDLIBFLAGS= $(addprefix -l,$(LDLIB))
STATICLIBS= /usr/lib/libatlas.so.3 /usr/lib/libf77blas.so.3 /usr/lib/libcblas.so.3 /usr/lib/liblapack_atlas.so.3

PYINTFILE= CpuCompute.so CpuInit.so

all: build

build:$(PYINTFILE)

$(PYINTFILE): %.so: %.i
	$(SWIG) $(SWIGFLAGS) $(INCLUDEFLAGS) $(MACROFLAGS) $<
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) $(MACROFLAGS) -c -o $(basename $@).o $(basename $@)_wrap.cxx
	$(CXX) -shared $(basename $@).o -o _$@ $(SOFLAGS) $(RUNLIBS) $(LIBSFLAGS) $(LDLIBFLAGS) $(STATICLIBS)

clean:
	@rm *.so *_wrap.cxx *.o

.PHONY: clean

