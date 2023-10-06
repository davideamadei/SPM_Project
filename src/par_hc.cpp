/**
 * @file seq_hc.cpp
 * @author Davide Amadei (davide.amadei97@gmail.com)
 * @brief file containing the parallel version of the huffman encoding using native c++ threads
 * @date 2023-10-04
 * 
 * Supports running multiple times for logging purposes 
 */
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <future>
#include <bits/stdc++.h>

#include "logger.hpp"
#include "huffman_tree.hpp"

using std::cout, std::clog, std::endl, std::string;

typedef struct{
    long time;
    char padding;
    std::shared_ptr<std::vector<char>> encoded_chunk;
} encoding_results;

void print_help(){
    cout << "The program accepts the following arguments:" << endl;
    cout << "\t -i path: path to the file to be encoded, required." << endl;
    cout << "\t -o path: path where the encoded file has to be saved, required." << endl;
    cout << "\t -n number: number of times to run the program, default 1." << endl;
    cout << "\t -t number: number of threads to use, default 4." << endl;
    cout << "\t -v: set verbose." << endl;
    cout << "\t -l: enable logging to file" << endl;
}

encoding_results encode_chunk(std::vector<std::pair<int,int>> &code_table, std::vector<char> &file_chunk){
    Timer timer;
    timer.start("encode");
    std::shared_ptr<std::vector<char>> buffer_vec (new std::vector<char>);
    // current size of the buffer
    int buf_len = 0;
    char buffer = 0;
    // remaining bits to save to buffer for currently considered character
    // is initialized with the length of the encoding for each character
    int remaining;
    // code of the character, stored as an int
    int code;
    // size of a single buffer
    int max_size = sizeof(char) * 8;

    // actual encoding of the file
    // encoding is stored into a vector of chars
    for(auto &c : file_chunk){
        auto code_pair = code_table[c];
        code = code_pair.second;
        remaining = code_pair.first;;

        // loop saving the encoding of the character to the buffer
        while(remaining != 0){
            // buffer has reached maximum size
            if(buf_len==max_size){
                // flush buffer to vector
                buffer_vec->push_back(buffer);
                // reset buffer
                buffer = 0;
                buf_len = 0;
            }
            // buffer has enough space to store the rest of the encoding
            // of the current character
            if(buf_len+remaining <= max_size){
                // shift buffer and add code to the end
                buffer = (buffer << remaining) | code;
                //update buffer size
                buf_len += remaining;
                remaining = 0;
            }
            // buffer does not have enough space to store the rest of the encoding
            // of the current character 
            else{
                int shift = max_size-buf_len;
                // shift buffer to left and shift encoding to right
                buffer = (buffer << shift) | (code >> (remaining - shift));
                remaining -= shift;
                buf_len = max_size;
            }
        }
    }
    // flush buffer if necessary and add padding bits to the end
    if(buf_len!=0){
        buffer_vec->push_back(buffer << (max_size-buf_len));
    }
    long time = timer.stop();
    encoding_results res;
    res.time = time;
    res.padding = max_size-buf_len;
    res.encoded_chunk = buffer_vec;
    return res;
}

int main(int argc, char* argv[]){

    string filename = "";
    string output_filename = "";
    int n_times = 1;
    int n_threads = 4;
    bool verbose = false;
    bool logs = false;

    // parse command line arguments
    int opt;

    while ((opt = getopt(argc, argv, "hi:o:n:t:vl")) != -1) {
        switch (opt) {
        case 'h':
            print_help();
            return 1;
        case 'i':
            filename = optarg;
            break;
        case 'o':
            output_filename = optarg;
            break;
        case 'n':
            n_times = atoi(optarg);
            break;
        case 't':
            n_threads = atoi(optarg);
            break;
        case 'v':
            verbose = true;
            break;
        case 'l':
            logs = true;
            break;
        default:
            print_help();
            return 0;
        }
    }

    if((filename == "") || (output_filename == "")){
        cout << "Input and output filenames are required." << endl;
        print_help();
        return 0;
    }

    string log_file = "./logs/par/" + std::to_string(n_threads) + "_" + filename;
    log_file = log_file.substr(0, log_file.find_last_of('.'))+".csv";

    Logger logger;
    Timer timer;
    long elapsed_time; 
    Timer tot_timer;

    // loop n_times
    for(int i = 0; i < n_times; i++){
        tot_timer.start("total");
        std::ifstream file(filename);

        std::stringstream file_buffer;
        // string file_str;

        int filesize = std::filesystem::file_size(filename);

        // vector of buffers to store the file in chunks
        std::vector<std::vector<char>> file_chunks(n_threads);
        int chunk_size = filesize / n_threads;

        // vector of vectors storing partial character counts
        std::vector<std::vector<int>> partial_counts(n_threads, std::vector<int>(128, 0));

        // vector storing thread ids
        std::vector<std::future<long>> count_tids;

        // function acting as body of thread to count characters of a chunk of file
        auto count_chars = [&partial_counts, &file_chunks](int tid){
            Timer timer;
            timer.start("freq_time");
            // auto chunk = file_chunks[tid];
            // auto count_vector = &partial_counts[tid];
            for(int i=0; i<file_chunks[tid].size(); i++){
                partial_counts[tid][file_chunks[tid][i]]++;
            }
            return timer.stop();
        };

        // time to read file, including the resizing of the buffer
        long read_time = 0;
        long freq_thread_overhead = 0;
        // read file
        logger.start("read_and_count");
            for(int i=0; i<n_threads; i++){
                timer.start("reading_input");
                if(i==n_threads-1){
                    file_chunks[i].resize(chunk_size + filesize % n_threads);
                    file.read(&(file_chunks[i])[0], chunk_size + filesize % n_threads);
                }
                else{
                    file_chunks[i].resize(chunk_size);
                    file.read(&(file_chunks[i])[0], chunk_size);
                }
                read_time += timer.stop();

                timer.start("freq_thread_overhead");
                count_tids.push_back(move(std::async(std::launch::async, count_chars, i)));
                freq_thread_overhead += timer.stop();
            }
            file.close();

            long freq_time = 0;
            for(auto &t : count_tids){
                freq_time += t.get();
            }

            // use array to store character counts
            // can be directly indexed using ASCII characters
            std::vector<int> count_vector(128, 0);

            timer.start("freq_join_overhead");
            for(auto &c : partial_counts){
                for(int i=0; i<c.size(); i++)
                {
                    count_vector[i] += c[i];
                }
            }
            long freq_join_overhead = timer.stop();
            
        elapsed_time = logger.stop();
        
        logger.add_stat("freq_thread_overhead", freq_thread_overhead);
        logger.add_stat("reading_input", read_time);
        logger.add_stat("freq_time", freq_time);
        logger.add_stat("freq_join_overhead", freq_join_overhead);

        if(verbose){
            cout << "Reading input and counting character frequency took " << elapsed_time << " real usecs." << endl;
            cout << "Reading input took " << read_time << " usecs." << endl;
            cout << "Counting characters took " << freq_time << " usecs in overall computation time between threads." << endl;
            cout << "Overhead for starting threads for frequency gathering was " << freq_thread_overhead << " usecs." << endl;
            cout << "Joining partial frequency counts took " << freq_join_overhead << " usecs." << endl;
        }
        
        // create the huffman tree and table of encodings
        logger.start("huffman_tree_creation");
            HuffmanTree ht(count_vector);
        elapsed_time = logger.stop();

        if(verbose){
            cout << "Creating the Huffman tree and extracting the code table took " << elapsed_time << " usecs." << endl;
        }

        auto code_table = ht.getCodes();

        long encode_time = 0;
        long write_time = 0;
        // encode and write to file in chunks
        logger.start("encode_and_write");

            std::vector<std::future<encoding_results>> encode_tids;
            
            timer.start("encode_thread_overhead");
            for(int i=0; i<n_threads; i++){
                encode_tids.push_back(move(std::async(std::launch::async, encode_chunk,
                                             std::ref(code_table), std::ref(file_chunks[i]))));
            }
            long encode_thread_overhead = timer.stop();

            timer.start("write");
                std::ofstream output_file(output_filename, std::ios::binary);  
                // write number of chunks          
                output_file.write(reinterpret_cast<const char *>(&n_threads), sizeof(n_threads));
                // write encoding table
                // more efficient ways exist
                for(auto &f : count_vector){
                    output_file.write(reinterpret_cast<const char *>(&f), sizeof(f));
                }
            write_time += timer.stop();

            for(int i=0; i<n_threads; i++){
                auto res = encode_tids[i].get();
                char ending_padding = res.padding;
                encode_time += res.time;
                auto buffer = res.encoded_chunk;

                int chunk_size = buffer->size();
                timer.start("write");
                // write size of chunk
                output_file.write(reinterpret_cast<const char *>(&chunk_size), sizeof(chunk_size));
                // write number of padding bits
                output_file.write(&ending_padding, 1);
                // write the encoded binary
                output_file.write(buffer->data(), buffer->size());
                write_time += timer.stop();

            }
        elapsed_time = logger.stop();
        logger.add_stat("write", write_time);
        logger.add_stat("encode", encode_time);
        logger.add_stat("encode_thread_overhead", encode_thread_overhead);

        if(verbose){
            cout << "Encoding and writing the file took " << elapsed_time << " usecs." << endl;
            cout << "Encoding the file took " << encode_time << " usecs." << endl;
            cout << "Writing encoded file took " << write_time << " usecs." << endl;
        }

        // if(verbose){
        //     cout << "Input file is " << filesize/8 << " bytes long" <<endl;
        //     cout<<endl<<endl;
        // }
        logger.add_stat("total", tot_timer.stop());
    }
    if(logs){
        std::filesystem::create_directory("./logs");
        std::filesystem::create_directory("./logs/par");
        logger.write_logs(log_file, n_times);
    }
    return 0;
}