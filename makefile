CXX = g++
CXXFLAGS = -std=c++20
CPPFLAGS = -pthread -O3

BUILDDIR = ./
SRCDIR = ./src
TESTDIR = ./test
ODIR = ./obj
UTILDIR = ./src/classes


.DEFAULT_GOAL := all
.PHONY : all

LIBS = $(UTILDIR)/logger.hpp $(UTILDIR)/huffman_tree.hpp

all: seq_hc.out decode_test.out

test: all 
	./seq_hc.out -i war-and-peace.txt -o war-and-peace.dat
	./decode_test.out war-and-peace.dat decoded-war-and-peace.txt >/dev/null 2>/dev/null
	
	diff war-and-peace.txt decoded-war-and-peace.txt
	./seq_hc.out -i commedia.txt -o commedia.dat
	./decode_test.out commedia.dat decoded-commedia.txt >/dev/null 2>/dev/null
	diff commedia.txt decoded-commedia.txt

	./seq_hc.out -i large-test.txt -o large-test.dat
	./decode_test.out large-test.dat decoded-large-test.txt >/dev/null 2>/dev/null
	diff large-test.txt decoded-large-test.txt

seq_hc.out: $(ODIR)/seq_hc.o $(LIBS) $(ODIR)/logger.o $(ODIR)/huffman_tree.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -I $(UTILDIR) -o $@ $< $(ODIR)/logger.o  $(ODIR)/huffman_tree.o

decode_test.out: $(ODIR)/decode_test.o $(LIBS) $(ODIR)/logger.o $(ODIR)/huffman_tree.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -I $(UTILDIR) -o $@ $< $(ODIR)/logger.o  $(ODIR)/huffman_tree.o

# $(ODIR)/seq_hc.o : $(SRCDIR)/seq_hc.cpp
# 	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -I $(UTILDIR) -I $(HTDIR) -o $@ $<

$(ODIR)/%.o: $(TESTDIR)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -I $(UTILDIR) -o $@ $<
	
$(ODIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -I $(UTILDIR) -o $@ $<

$(ODIR)/%.o: $(UTILDIR)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -I $(UTILDIR) -o $@ $<

$(ODIR)/%.o: $(HTDIR)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -I $(UTILDIR) -o $@ $<

download_txt: commedia.txt war-and-peace.txt
	wget -O tmp.txt -c https://dmf.unicatt.it/~della/pythoncourse18/commedia.txt
	iconv -f UTF-8 -t ASCII//TRANSLIT -c tmp.txt > commedia.txt
	rm tmp.txt
	wget -O tmp.txt -c https://raw.githubusercontent.com/mmcky/nyu-econ-370/master/notebooks/data/book-war-and-peace.txt
	iconv -f UTF-8 -t ASCII//TRANSLIT -c tmp.txt > war-and-peace.txt
	rm tmp.txt

docs:
	doxygen DOXYGEN

clean:
	rm -rf $(ODIR)/*.o

cleaner: clean
	rm -rf *.out *.dat decoded* html/* latex/*

# does not actually work, just used to store command
create_large_test:
	for i in {1..40}; do cat war-and-peace.txt >> large-test.txt; done