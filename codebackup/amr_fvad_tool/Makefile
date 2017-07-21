CXX=g++
AR=ar
RANLIB=ranlib

ROOTPATH=$(CURDIR)

CXXFLAGS= -g -Wall -O2 -std=c++11 -Wpedantic -DHAVE_CONFIG_H
CFLAGS=

#first install opencore-amr
#then install libfvad and libsndfile
#modify these path

FVADROOT=/home/gzdinghanyu/vad
AMRROOT=/home/gzdinghanyu/amr

INCLUDEDIR= . $(AMRROOT)/include/opencore-amrwb $(FVADROOT)/include
INCLUDEDIRFLAGS = $(addprefix -I,$(INCLUDEDIR))
LIBDIR = $(AMRROOT)/lib -L$(AMRROOT)/lib -Wl,-rpath,$(FVADROOT)/lib -L$(FVADROOT)/lib
LDLIBS= m opencore-amrwb fvad
LDLIBSFLAGS = $(addprefix -l,$(LDLIBS))
STATICLIBS = $(AMRROOT)/lib/libopencore-amrwb.a
MYSTATICLIBS= mylib/libmy.a

BINFILES = amrwbPipe amrwbPipeWithVad
OBJFILES = fvadfunc.o wavwriter.o  $(addsuffix .o,$(BINFILES))

EXTERNALOBJ=

all: objbuild binbuild

makesubdir:
	make -C $(ROOTPATH)/src

objbuild: $(EXTERNALOBJ) $(OBJFILES) generateLib

binbuild: objbuild $(EXTERNALOBJ) $(BINFILES)

$(EXTERNALOBJ): %.o: %.c
	$(CXX) $(CXXFLAGS) $(INCLUDEDIRFLAGS) -c $< -o $@

$(OBJFILES): %.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDEDIRFLAGS) -c $< -o $@

generateLib: $(OBJFILES)
	test -d $(ROOTPATH)/mylib || mkdir -p $(ROOTPATH)/mylib || exit 1
	$(AR) -cr libmy.a $(OBJFILES) $(EXTERNALOBJ)
	$(RANLIB) libmy.a
	mv libmy.a $(ROOTPATH)/mylib

$(BINFILES): $(MYSTATICLIBS)
	$(CXX) $(CXXFLAGS) $(INCLUDEDIRFLAGS) -o $@ $@.o $(MYSTATICLIBS) -Wl,-rpath,$(LIBDIR) $(LDLIBSFLAGS) $(STATICLIBS)

clean:
	@rm -f $(BINFILES) *.o *.a
	@rm -rf $(ROOTPATH)/mylib


.PHONY: clean all objbuild binbuild

