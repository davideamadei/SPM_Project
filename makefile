CXX = g++
CXXFLAGS = -std=c++20
CPPFLAGS = -pthread -O3

vpath %.cpp src

SRCDIR = ./src
TESTDIR = ./test
ODIR = ./obj
UTILDIR = ./src/utils
HTDIR = ./src/huffman_tree
EXEDIR = ./bin

SOURCES=$(shell find . -name $(SRCDIR)/*.cpp)
TESTSRCS=$(shell find . -name $(TESTDIR)/*.cpp)
# OBJECTS=$(SOURCES:%.cpp=$(ODIR)/%.o)
# OBJECTS+=$(TESTDIR:%.cpp=$(ODIR)/%.o)
# OBJECTS+=$(UTILDIR:%.cpp=$(ODIR)/%.o)
# OBJECTS+=$(HTDIR:%.cpp=$(ODIR)/%.o)


.DEFAULT_GOAL := all
.PHONY : all

LIBS = $(UTILDIR)/utimer.hpp $(UTILDIR)/utils.hpp $(HTDIR)/huffman_tree.hpp

all: $(EXEDIR)/seq_hc.out 

test: $(EXEDIR)/read_test.out

$(EXEDIR)/seq_hc.out: $(ODIR)/seq_hc.o $(LIBS) $(UTILDIR)/utils.o $(HTDIR)/huffman_tree.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -I $(UTILDIR) -I $(HTDIR) -o $@ $< $(UTILDIR)/utils.o  $(HTDIR)/huffman_tree.o

$(EXEDIR)/read_test.out: $(TESTDIR)/read_test.cpp $(LIBS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -I $(UTILDIR) -I $(HTDIR) -o "$(EXEDIR)/$@" $<

$(ODIR)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -I $(UTILDIR) -I $(HTDIR) -o $@ $<

%.out: $(ODIR)/%.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

download_txt:
	wget -O tmp.txt -c https://dmf.unicatt.it/~della/pythoncourse18/commedia.txt
	iconv -f UTF-8 -t ASCII//TRANSLIT -c tmp.txt > commedia.txt
	rm tmp.txt
	wget -O tmp.txt -c https://raw.githubusercontent.com/mmcky/nyu-econ-370/master/notebooks/data/book-war-and-peace.txt
	iconv -f UTF-8 -t ASCII//TRANSLIT -c tmp.txt > war-and-peace.txt
	rm tmp.txt

clean:
	rm $(ODIR)/*.o

