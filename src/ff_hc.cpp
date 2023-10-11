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
#include <ff/ff.hpp>
#include <ff/farm.hpp>
#include <ff/parallel_for.hpp>
#include "logger.hpp"
#include "huffman_tree.hpp"

using std::cout, std::clog, std::endl, std::string, std::vector, std::shared_ptr;
using namespace ff;

std::mutex m;
std::condition_variable cv;
int write_id = 0;

void print_help(){
    cout << "The program accepts the following arguments:" << endl;
    cout << "\t -i path: path to the file to be encoded, required." << endl;
    cout << "\t -o path: path where the encoded file has to be saved, required." << endl;
    cout << "\t -n number: number of times to run the program, default 1." << endl;
    cout << "\t -t number: number of threads to use, default 4." << endl;
    cout << "\t -v: set verbose." << endl;
    cout << "\t -l: enable logging to file" << endl;
}

class Reader : public ff_monode_t<int>{
public:
    int n_workers;
    vector<shared_ptr<vector<char>>> file_chunks;
    string filename;
    Reader(int n_workers, vector<shared_ptr<vector<char>>> file_chunks, string filename){
        this->filename = filename;
        this->n_workers = n_workers;
        this->file_chunks = file_chunks;
    }
    int * svc(int * in){
        Timer timer;
        timer.start("reading");
        std::ifstream file(filename);

        std::stringstream file_buffer;

        auto filesize = file.tellg();
        file.seekg( 0, std::ios::end );
        filesize = file.tellg() - filesize;
        file.clear();
        file.seekg(0, std::ios::beg);

        // vector of buffers to store the file in chunks
        auto chunk_size = filesize / n_workers;
        for(int i=0; i<n_workers; i++){
            if(i==n_workers-1){
                file_chunks[i]->resize(chunk_size + filesize % n_workers);
                file.read(&(*(file_chunks[i]))[0], chunk_size + filesize % n_workers);
                ff_send_out_to(new int(i), i%n_workers);
            }
            else{
                file_chunks[i]->resize(chunk_size);
                file.read(&(*(file_chunks[i]))[0], chunk_size);
                ff_send_out(new int(i), i%n_workers);
            }
        }
        timer.stop();
        file.close();
        return EOS;
    }

};

class freqTask : public ff_node_t<int>{
public:
    shared_ptr<vector<int>> partial_counts;
    shared_ptr<vector<char>> file_chunk;
    freqTask(shared_ptr<vector<int>> partial_counts, shared_ptr<vector<char>> file_chunk){
        this->partial_counts = partial_counts;
        this->file_chunk = file_chunk;
    }
    int * svc(int * i ){
        // auto chunk = file_chunks[tid];
        // auto count_vector = &partial_counts[tid];
        Timer timer;
        timer.start("freq");
        for(int j=0; j<file_chunk->size(); j++){
            (*partial_counts)[(*file_chunk)[j]]++;
        }
        free(i);
        timer.stop();
        return i;
    }

};


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

    string log_file = "./logs/ff/" + std::to_string(n_threads) + "_" + filename;
    log_file = log_file.substr(0, log_file.find_last_of('.'))+".csv";

    Logger logger;
    Timer timer;
    long elapsed_time; 
    Timer tot_timer;

    // loop n_times
    for(int i = 0; i < n_times; i++){
        tot_timer.start("total");
        write_id=0;

        // vector of vectors storing partial character counts
        vector<shared_ptr<vector<int>>> partial_counts(n_threads);
        for(int i=0; i<partial_counts.size(); i++){
            partial_counts[i] = std::make_shared<vector<int>>(128);
        }

        vector<int> count_vector(128);
        // time to read file, including the resizing of the buffer
        long read_time = 0;

        vector<shared_ptr<vector<char>>> file_chunks(n_threads);
        for(int i=0; i<file_chunks.size(); i++){
            file_chunks[i] = std::make_shared<vector<char>>();
        }

        logger.start("read_and_count");
            // std::unique_ptr<Reader> read_node = make_unique<Reader>(n_threads, file_chunks, filename);
            Reader read_node(n_threads, file_chunks, filename);
            vector<std::unique_ptr<ff_node>> workers;
            for(int i=0; i<n_threads; i++){
                workers.push_back(make_unique<freqTask>(partial_counts[i], file_chunks[i]));
            }
            ff_Farm<freqTask> freq_farm(move(workers), read_node);
            // ff_Pipe<> freq_pipe(read_node, freq_farm);
            freq_farm.remove_collector();            
            if (freq_farm.run_and_wait_end()<0) {
                error("running farm\n");
                return -1;
            }
        timer.start("freq_join_overhead");
        for(auto &c : partial_counts){
            for(int i=0; i<c->size(); i++)
            {
                count_vector[i] += (*c)[i];
            }
        }
        long freq_join_overhead = timer.stop();
            
        elapsed_time = logger.stop();
        
        logger.add_stat("reading_input", read_time);
        // logger.add_stat("freq_time", freq_time);
        logger.add_stat("freq_join_overhead", freq_join_overhead);

        if(verbose){
            cout << "Reading input and counting character frequency took " << elapsed_time << " real usecs." << endl;
            cout << "Reading input took " << read_time << " usecs." << endl;
            // cout << "Counting characters took " << freq_time << " usecs in overall computation time between threads." << endl;
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

        vector<long> write_time_vec(n_threads);
        vector<long> encode_time_vec(n_threads);

        // encode and write to file in chunks

        logger.start("encode_and_write");

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

            auto encode_chunk = [&](int i) {
                Timer timer_encode;
                timer_encode.start("encode");

                vector<char> buffer_vec;

                // current size of the buffer
                int buf_len = 0;
                char buffer = 0;
                // remaining bits to save to buffer for currently considered character
                // is initialized with the length of the encoding for each character
                int remaining;
                // code of the character, stored as an int
                int code;
                // size of a single buffer
                const int max_size = sizeof(char) * 8;

                // actual encoding of the file
                // encoding is stored into a vector of chars
                for(auto &c : *(file_chunks)[i]){
                    auto code_pair = code_table[c];
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
                // flush buffer if necessary and add padding bits to the end
                if(buf_len!=0){
                    buffer_vec.push_back(buffer << (max_size-buf_len));
                }
                encode_time_vec[i] = timer_encode.stop();

                int chunk_size = buffer_vec.size();
                char padding = max_size-buf_len;
                std::unique_lock lk(m);
                while(i != write_id){
                    cv.wait(lk);
                }
                timer_encode.start("write");
                // write size of chunk
                output_file.write(reinterpret_cast<const char *>(&chunk_size), sizeof(chunk_size));
                // write number of padding bits
                output_file.write(&padding, 1);
                // write the encoded binary
                output_file.write(buffer_vec.data(), buffer_vec.size());

                write_time_vec[i] =  timer.stop();;
                write_id++;
                cv.notify_all();
                return;
            };

            ParallelFor encode_pf(n_threads);

            encode_pf.parallel_for_static(0, n_threads, 1, 0, encode_chunk, n_threads);

        elapsed_time = logger.stop();
        for(auto &t : encode_time_vec){
            encode_time += t;
        }

        for(auto &t : write_time_vec){
            write_time += t;
        }
        
        logger.add_stat("write", write_time);
        logger.add_stat("encode", encode_time);

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
        std::filesystem::create_directory("./logs/ff");
        logger.write_logs(log_file, n_times);
    }
    return 0;
}