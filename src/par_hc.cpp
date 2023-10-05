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

void print_help(){
    cout << "The program accepts the following arguments:" << endl;
    cout << "\t -i path: path to the file to be encoded, required." << endl;
    cout << "\t -o path: path where the encoded file has to be saved, required." << endl;
    cout << "\t -n number: number of times to run the program, default 1." << endl;
    cout << "\t -t number: number of threads to use, default 4." << endl;
    cout << "\t -v: set verbose." << endl;
    cout << "\t -l: enable logging to file" << endl;
}

// std::pair<char, long> encode_chunk(std::vector<std::pair<int,int>> code_table, std::vector<char> file_chunk){
//     Timer timer;

// }

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
    long elapsed_time; 

    // loop n_times
    for(int i = 0; i < n_times; i++){
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

        Timer timer;
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
        

        // if(verbose){
        //     cout << endl;

        //     clog << 10 << ", " << "LF" << ", " << count_vector[10] << endl;
        //     clog << 13 << ", " << "CR" << ", " << count_vector[13] << endl;

        //     // print character counts
        //     for(int i=32; i<127; i++){
        //         clog << i << ", " << char(i) << ", " << count_vector[i] << endl;
        //     }
        //     cout << endl;
        // }
        
        // create the huffman tree and table of encodings
        logger.start("huffman_tree_creation");
            HuffmanTree ht(count_vector);
        elapsed_time = logger.stop();

        if(verbose){
            cout << "Creating the Huffman tree and extracting the code table took " << elapsed_time << " usecs." << endl;
        }

        auto code_table = ht.getCodes();


        std::vector<char> buffer_vec;
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

        // there for consistency with parallel version
        int n_chunks = 1;

        // actual encoding of the file
        // encoding is stored into a vector of chars
        logger.start("encoding");
        for(auto &c : file_chunks){
            for(int i=0; i<c.size(); i++){
                auto code_pair = code_table[c[i]];
                code = code_pair.second;
                remaining = code_pair.first;;

                // loop saving the encoding of the character to the buffer
                while(remaining != 0){
                    // buffer has reached maximum size
                    if(buf_len==max_size){
                        // flush buffer to vector
                        buffer_vec.push_back(buffer);
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
        }
            // flush buffer if necessary and add padding bits to the end
            if(buf_len!=0){
                buffer_vec.push_back(buffer << (max_size-buf_len));
            }
        elapsed_time = logger.stop();

        if(verbose){
            cout << "Encoding the file took " << elapsed_time << " usecs." << endl;
        }
        
        // number of bits of padding required
        char ending_padding = max_size - buf_len;

        // number of buffers required to store the encoding
        int chunk_byte_size = buffer_vec.size();

        std::ofstream output_file(output_filename, std::ios::binary);

        logger.start("writing_output");
            // write number of chunks
            output_file.write(reinterpret_cast<const char *>(&n_chunks), sizeof(n_chunks));

            // write encoding table
            // more efficient ways exist
            for(auto &f : count_vector){
                output_file.write(reinterpret_cast<const char *>(&f), sizeof(f));
            }

            // write size of chunk
            output_file.write(reinterpret_cast<const char *>(&chunk_byte_size), sizeof(chunk_byte_size));
            // write number of padding bits
            output_file.write(&ending_padding, 1);
            // write the encoded binary
            output_file.write(buffer_vec.data(), buffer_vec.size());
        elapsed_time = logger.stop();

        if(verbose){
            cout << "Writing encoded file took " << elapsed_time << " usecs." << endl;
        }

        if(verbose){
            cout << "Input file is " << filesize/8 << " bytes long" <<endl;
            cout << "Encoded file is " << chunk_byte_size << " bytes long" << endl;
            cout<<endl<<endl;
        }
        
    }
    if(logs){
        std::filesystem::create_directory("./logs");
        std::filesystem::create_directory("./logs/par");
        logger.write_logs(log_file, n_times);
    }
    return 0;
}