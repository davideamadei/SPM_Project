CXX = g++
CXXFLAGS = -std=c++20
CPPFLAGS = -pthread -O3

BUILDDIR = ./
SRCDIR = ./src
TESTDIR = ./test
ODIR = ./obj
UTILDIR = ./src/classes
IDIR = ./lib

INCLUDES = -I $(UTILDIR) -I $(IDIR)

.DEFAULT_GOAL := all
.PHONY : all

LIBS = $(UTILDIR)/logger.hpp $(UTILDIR)/huffman_tree.hpp

all: seq_hc.out decode_test.out par_hc.out ff_hc.out

seq_hc.out: $(ODIR)/seq_hc.o $(LIBS) $(ODIR)/logger.o $(ODIR)/huffman_tree.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $< $(ODIR)/logger.o  $(ODIR)/huffman_tree.o

par_hc.out: $(ODIR)/par_hc.o $(LIBS) $(ODIR)/logger.o $(ODIR)/huffman_tree.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $< $(ODIR)/logger.o  $(ODIR)/huffman_tree.o

par_hc2.out: $(ODIR)/par_hc2.o $(LIBS) $(ODIR)/logger.o $(ODIR)/huffman_tree.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $< $(ODIR)/logger.o  $(ODIR)/huffman_tree.o

ff_hc.out : $(ODIR)/ff_hc.o $(LIBS) $(ODIR)/logger.o $(ODIR)/huffman_tree.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $< $(ODIR)/logger.o  $(ODIR)/huffman_tree.o

decode_test.out: $(ODIR)/decode_test.o $(LIBS) $(ODIR)/logger.o $(ODIR)/huffman_tree.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $< $(ODIR)/logger.o  $(ODIR)/huffman_tree.o


$(ODIR)/%.o: $(TESTDIR)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $<
	
$(ODIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $<

$(ODIR)/%.o: $(UTILDIR)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $<

$(ODIR)/%.o: $(HTDIR)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $<


download_txt: commedia.txt war-and-peace.txt
	wget -O tmp.txt -c https://dmf.unicatt.it/~della/pythoncourse18/commedia.txt
	iconv -f UTF-8 -t ASCII//TRANSLIT -c tmp.txt > commedia.txt
	rm tmp.txt
	wget -O tmp.txt -c https://raw.githubusercontent.com/mmcky/nyu-econ-370/master/notebooks/data/book-war-and-peace.txt
	iconv -f UTF-8 -t ASCII//TRANSLIT -c tmp.txt > war-and-peace.txt
	rm tmp.txt


test_seq: all 
	./seq_hc.out -i war-and-peace.txt -o war-and-peace.dat -v
	./decode_test.out war-and-peace.dat decoded-war-and-peace.txt >/dev/null 2>/dev/null
	diff war-and-peace.txt decoded-war-and-peace.txt
	
	./seq_hc.out -i commedia.txt -o commedia.dat -v
	./decode_test.out commedia.dat decoded-commedia.txt >/dev/null 2>/dev/null
	diff commedia.txt decoded-commedia.txt

large_test_seq:
	./seq_hc.out -i large-test.txt -o large-test.dat -v
	./decode_test.out large-test.dat decoded-large-test.txt >/dev/null 2>/dev/null
	diff large-test.txt decoded-large-test.txt

test_par: all 
	./par_hc.out -i war-and-peace.txt -o war-and-peace.dat -v
	./decode_test.out war-and-peace.dat decoded-war-and-peace.txt >/dev/null 2>/dev/null
	diff war-and-peace.txt decoded-war-and-peace.txt
	
	./par_hc.out -i commedia.txt -o commedia.dat -v
	./decode_test.out commedia.dat decoded-commedia.txt >/dev/null 2>/dev/null
	diff commedia.txt decoded-commedia.txt

large_test_par:
	./par_hc.out -i large-test.txt -o large-test.dat -v
	./decode_test.out large-test.dat decoded-large-test.txt >/dev/null 2>/dev/null
	diff large-test.txt decoded-large-test.txt

test_ff: all 
	./ff_hc.out -i war-and-peace.txt -o war-and-peace.dat -v
	./decode_test.out war-and-peace.dat decoded-war-and-peace.txt >/dev/null 2>/dev/null
	diff war-and-peace.txt decoded-war-and-peace.txt
	
	./ff_hc.out -i commedia.txt -o commedia.dat -v
	./decode_test.out commedia.dat decoded-commedia.txt >/dev/null 2>/dev/null
	diff commedia.txt decoded-commedia.txt

docs:
	doxygen DOXYGEN

clean:
	rm -rf $(ODIR)/*.o

cleaner: clean
	rm -rf *.out *.dat decoded* html/* latex/*

# does not actually work, just used to store command
create_large_test:
	for i in {1..40}; do cat war-and-peace.txt >> large-test.txt; done