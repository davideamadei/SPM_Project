SHELL := /bin/bash
CXX = g++
CXXFLAGS = -std=c++20
CXXFLAGS_FF = -std=c++17
CPPFLAGS = -pthread -O3

BUILDDIR = ./
SRCDIR = ./src
TESTDIR = ./test
ODIR = ./obj
UTILDIR = ./src/classes
IDIR = ./lib

INCLUDES = -I $(UTILDIR) -I $(IDIR)

.DEFAULT_GOAL := all
.PHONY : all logs

LIBS = $(UTILDIR)/logger.hpp $(UTILDIR)/huffman_tree.hpp

all: seq_hc.out decode_test.out par_hc.out ff_hc.out


# rules to make executables

seq_hc.out: $(ODIR)/seq_hc.o $(LIBS) $(ODIR)/logger.o $(ODIR)/huffman_tree.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $< $(ODIR)/logger.o  $(ODIR)/huffman_tree.o

par_hc.out: $(ODIR)/par_hc.o $(LIBS) $(ODIR)/logger.o $(ODIR)/huffman_tree.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $< $(ODIR)/logger.o  $(ODIR)/huffman_tree.o

ff_hc.out : $(ODIR)/ff_hc.o $(LIBS) $(ODIR)/logger.o $(ODIR)/huffman_tree.o
	$(CXX) $(CXXFLAGS_FF) $(CPPFLAGS) $(INCLUDES) -o $@ $< $(ODIR)/logger.o  $(ODIR)/huffman_tree.o

decode_test.out: $(ODIR)/decode_test.o $(LIBS) $(ODIR)/logger.o $(ODIR)/huffman_tree.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $< $(ODIR)/logger.o  $(ODIR)/huffman_tree.o


# generic rules to compile object files

$(ODIR)/%.o: $(TESTDIR)/%.cpp
	@mkdir -p $(ODIR)
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $<
	
$(ODIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(ODIR)
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $<

$(ODIR)/%.o: $(UTILDIR)/%.cpp
	@mkdir -p $(ODIR)
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $<

$(ODIR)/%.o: $(HTDIR)/%.cpp
	@mkdir -p $(ODIR)
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $<


# download the two test files and convert them to ASCII
download_txt: 
	wget -O tmp.txt -c https://dmf.unicatt.it/~della/pythoncourse18/commedia.txt
	iconv -f UTF-8 -t ASCII//TRANSLIT -c tmp.txt > commedia.txt
	rm tmp.txt
	wget -O tmp.txt -c https://raw.githubusercontent.com/mmcky/nyu-econ-370/master/notebooks/data/book-war-and-peace.txt
	iconv -f UTF-8 -t ASCII//TRANSLIT -c tmp.txt > war-and-peace.txt
	rm tmp.txt


# tests of correctness, encode the test files and decode them
# the ones using the large file are very slow because of the decoding process
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

large_test_ff:
	./ff_hc.out -i large-test.txt -o large-test.dat -v
	./decode_test.out large-test.dat decoded-large-test.txt >/dev/null 2>/dev/null
	diff large-test.txt decoded-large-test.txt


# rules to run the program in its various versions and make logs for varying amounts of threads

numbers = {1..64}

logs_seq: all
	for i in {1..10}; do \
		./seq_hc.out -i war-and-peace.txt -o war-and-peace.dat -l logs; \
	done 

logs_par: all
	for i in {1..10}; do \
		for number in $(numbers) ; do \
			./par_hc.out -i war-and-peace.txt -o war-and-peace.dat -l logs -t $$number; \
		done; \
	done

logs_ff: all
	for i in {1..10}; do \
		for number in $(numbers) ; do \
			./ff_hc.out -i war-and-peace.txt -o war-and-peace.dat -l logs -t $$number; \
		done; \
	done

logs_large_seq: all
	for i in {1..10}; do \
		./seq_hc.out -i large-test.txt -o large-test.dat -l logs;  \
	done

logs_large_par: all
	for i in {1..10}; do \
		for number in $(numbers) ; do \
			./par_hc.out -i large-test.txt -o large-test.dat -l logs -t $$number; \
		done; \
	done

logs_large_ff: all
	for i in {1..10}; do \
		for number in $(numbers) ; do \
			./ff_hc.out -i large-test.txt -o large-test.dat -l logs -t $$number; \
		done; \
	done


# logs without logging read and write times with frequency gathering and encoding

logs_no_rw_par: all
	for i in {1..10}; do \
		for number in $(numbers) ; do \
			./par_hc.out -i war-and-peace.txt -l logs_no_rw -t $$number -d; \
		done; \
	done

logs_no_rw_ff: all
	for i in {1..10}; do \
		for number in $(numbers) ; do \
			./ff_hc.out -i war-and-peace.txt -l logs_no_rw -t $$number -d; \
		done; \
	done

logs_large_no_rw_par: all
	for i in {1..10}; do \
		for number in $(numbers) ; do \
			./par_hc.out -i large-test.txt -l logs_no_rw -t $$number -d; \
		done; \
	done

logs_large_no_rw_ff: all
	for i in {1..10}; do \
		for number in $(numbers) ; do \
			./ff_hc.out -i large-test.txt -l logs_no_rw -t $$number -d; \
		done; \
	done


# logs shortcuts
logs:
	make logs_seq
	make logs_par
	make logs_ff
	make logs_no_rw_par
	make logs_no_rw_ff

large_logs:
	make logs_large_seq
	make logs_large_par
	make logs_large_ff
	make logs_large_no_rw_par
	make logs_large_no_rw_ff


# make docs

docs:
	doxygen DOXYGEN


# clean directory
clean:
	rm -rf $(ODIR)/

cleaner: clean
	rm -rf *.out *.dat decoded* html/ latex/

create_large_test:
	for i in {1..40}; do \
		cat war-and-peace.txt; \
	done >> large-test.txt