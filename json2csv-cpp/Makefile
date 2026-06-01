CXX = g++
CXXFLAG = -O2 -std=c++14 -Wall
LFLAG = -l

LIBDIR0 = ../jsoncpplib/lib
LIBDIRS = -L${LIBDIR0}

INC0 = ${HOME}/json2csv-cpp/
INC1 = ../jsoncpplib/include
INC2 = /usr/include/
INCDIRS = -I${INC0} -I${INC1} -I${INC2}

LIBJSONCPP = -ljsoncpp
LDFLAGS   = $(LIBJSONCPP) -Wl,-rpath,${LIBDIR0}

DEBUGF = -g -D DEBUG
DEBUG = no

BIN=json2csv

SRC=$(wildcard *.cpp)
OBJ=$(SRC:%.cpp=%.o)

.SUFFIXES: .exe .CXX
.CXX.exe: $(CXX) $(CXXFLAG) $@ $< $@

.PHONY: clean

json2csv: $(SRC) 
	$(CXX) $(CXXFLAG) $(INCDIRS) -o $(BIN) $^ $(LIBDIRS) $(LDFLAGS)

clean: 
	rm -f *.o
	rm $(BIN)
