#
#
#

CXX = g++
#CXXFLAGS = -Wall -g -O
CXXFLAGS = -std=c++11 -Wall -g -O
LDFLAGS =

ROOTINCDIR = $(shell $(ROOTSYS)/bin/root-config --incdir)
ROOTLIBDIR = $(shell $(ROOTSYS)/bin/root-config --libdir)
#ROOTLIBS = $(shell $(ROOTSYS)/bin/root-config --libs) \
#	-lNew -Xlinker -rpath $(ROOTSYS)/lib
ROOTLIBS = $(shell $(ROOTSYS)/bin/root-config --libs) \
	-Xlinker -rpath $(ROOTSYS)/lib
ROOTGLIBS = $(shell $(ROOTSYS)/bin/root-config --glibs)

EXECS = appmain decode mperiod qanal

all: $(EXECS)

decode: decode.cxx
	$(CXX) $(CXXFLAGS) -o $@ -D TEST_MAIN \
		$<

mperiod: mperiod.cxx
	$(CXX) $(CXXFLAGS) -o $@ -D PERIOD_TEST_MAIN \
		$<

onlinedisplay: onlinedisplay.cxx
	$(CXX) $(CXXFLAGS) -o $@ \
		-I$(ROOTINCDIR) \
		onlinedisplay.cxx \
		-L$(ROOTLIBDIR) $(ROOTLIBS) $(ROOTGLIBS) \
		-lThread \
		-Wl,-rpath=$(ROOTSYS)/lib

qanal: qanal.cxx decode.cxx
	$(CXX) $(CXXFLAGS) -o $@ \
		-I$(ROOTINCDIR) \
		$< \
		-L$(ROOTLIBDIR) $(ROOTLIBS) $(ROOTGLIBS) \
		-lThread \
		-Wl,-rpath=$(ROOTSYS)/lib

r6test: r6test.cxx
	$(CXX) $(CXXFLAGS) -o $@ \
		-I$(ROOTINCDIR) \
		r6test.cxx \
		$(ROOTLIBS) $(ROOTGLIBS) \
		-lRHTTP
		#-lThread

appmain: appmain.cxx appframe.h appframe.cxx appframe_dict.cxx datareader.cxx
	$(CXX) $(CXXFLAGS) -o $@ \
		-I$(ROOTINCDIR) \
		appmain.cxx appframe.cxx appframe_dict.cxx \
		-L$(ROOTLIBDIR) $(ROOTLIBS) $(ROOTGLIBS) \
		-lThread

appframe_dict.cxx: appframe.h appframe_LinkDef.h
	rootcling -f $@ -c  -p $^


tstexec: tstexec.cxx
	$(CXX) $(CXXFLAGS) -o $@ \
		-DSTANDALONE \
		-I$(ROOTINCDIR) \
		$< \
		-L$(ROOTLIBDIR) $(ROOTLIBS) $(ROOTGLIBS) \
		-lThread

tstmain: tstmain.cxx tst.cxx tst.h tstdict.cxx
	$(CXX) $(CXXFLAGS) -o $@ \
		-DSTANDALONE \
		-I$(ROOTINCDIR) \
		tstmain.cxx tst.cxx tstdict.cxx \
		-L$(ROOTLIBDIR) $(ROOTLIBS) $(ROOTGLIBS) \
		-lThread

tstmaindl: tstmain.cxx tst.cxx tstdict.cxx
	$(CXX) $(CXXFLAGS) -o $@ \
		-DSTANDALONE \
		-I$(ROOTINCDIR) \
		tstmain.cxx -L. -ltst \
		-L$(ROOTLIBDIR) $(ROOTLIBS) $(ROOTGLIBS) \
		-lThread

tstdict.cxx: tst.h LinkDef.h
	#rootcling -f $@ -c $(CXXFLAGS) -p $^
	rootcling -f $@ -c  -p $^

libtst.so: tstdict.cxx tst.cxx tst.h
	#$(CXX) -shared -fPIC -o$@ `root-config --ldflags` $(CXXFLAGS) -I$(ROOTSYS)/include $^
	$(CXX) -shared -fPIC -o$@ -m64 $(CXXFLAGS) -I$(ROOTSYS)/include $^

clean:
	rm -f $(EXECS)
	rm -f appframe_dict.cxx appframe_dict_rdict.pcm
	rm -f decode mperiod
	rm -f onlinedisplay
	rm -f display.png display.pdf
