CXX = g++
CXXFLAGS = -std=c++20
CPPFLAGS = -pthread -O3

BUILDDIR = ./
SRCDIR = ./src
TESTDIR = ./test
ODIR = ./obj
UTILDIR = ./src/utils
HTDIR = ./src/huffman_tree


.DEFAULT_GOAL := all
.PHONY : all

LIBS = $(UTILDIR)/utimer.hpp $(UTILDIR)/utils.hpp $(HTDIR)/huffman_tree.hpp

all: seq_hc.out decode_test.out

download_txt: commedia.txt war-and-peace.txt
	wget -O tmp.txt -c https://dmf.unicatt.it/~della/pythoncourse18/commedia.txt
	iconv -f UTF-8 -t ASCII//TRANSLIT -c tmp.txt > commedia.txt
	rm tmp.txt
	wget -O tmp.txt -c https://raw.githubusercontent.com/mmcky/nyu-econ-370/master/notebooks/data/book-war-and-peace.txt
	iconv -f UTF-8 -t ASCII//TRANSLIT -c tmp.txt > war-and-peace.txt
	rm tmp.txt

test: all 
	./seq_hc.out war-and-peace.txt encoded-war-and-peace.dat >/dev/null 2>/dev/null
	./decode_test.out encoded-war-and-peace.dat decoded-war-and-peace.txt >/dev/null 2>/dev/null
	diff war-and-peace.txt decoded-war-and-peace.txt
	./seq_hc.out commedia.txt encoded-commedia.dat >/dev/null 2>/dev/null
	./decode_test.out encoded-commedia.dat decoded-commedia.txt >/dev/null 2>/dev/null
	diff commedia.txt decoded-commedia.txt


seq_hc.out: $(ODIR)/seq_hc.o $(LIBS) $(ODIR)/utils.o $(ODIR)/huffman_tree.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -I $(UTILDIR) -I $(HTDIR) -o $@ $< $(ODIR)/utils.o  $(ODIR)/huffman_tree.o

decode_test.out: $(ODIR)/decode_test.o $(LIBS) $(ODIR)/utils.o $(ODIR)/huffman_tree.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -I $(UTILDIR) -I $(HTDIR) -o $@ $< $(ODIR)/utils.o  $(ODIR)/huffman_tree.o

# $(ODIR)/seq_hc.o : $(SRCDIR)/seq_hc.cpp
# 	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -I $(UTILDIR) -I $(HTDIR) -o $@ $<

$(ODIR)/%.o: $(TESTDIR)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -I $(UTILDIR) -I $(HTDIR) -o $@ $<
	
$(ODIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -I $(UTILDIR) -I $(HTDIR) -o $@ $<

$(ODIR)/%.o: $(UTILDIR)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -I $(UTILDIR) -I $(HTDIR) -o $@ $<

$(ODIR)/%.o: $(HTDIR)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -I $(UTILDIR) -I $(HTDIR) -o $@ $<

%.out: $(ODIR)/%.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $<



clean:
	rm -rf $(ODIR)/*.o

cleaner: clean
	rm -rf *.out *.dat decoded*
