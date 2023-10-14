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
#include <mutex>
#include <condition_variable>
#include "logger.hpp"
#include "huffman_tree.hpp"

using std::cout, std::clog, std::endl, std::string, std::vector, std::shared_ptr;

int write_id = 0;

typedef struct{
    long encode_time;
    long write_time;
} encoding_results;

void print_help(){
    cout << "The program accepts the following arguments:" << endl;
    cout << "\t -i path: path to the file to be encoded, required." << endl;
    cout << "\t -o path: path where the encoded file has to be saved, required." << endl;
    cout << "\t -t number: number of threads to use, default 4." << endl;
    cout << "\t -v: set verbose." << endl;
    cout << "\t -l dir: enable logging to file, output is written to directory dir." << endl;
    cout << "\t -d: debug mode, only works if logging is enabled." << endl;
}


/**
 * @brief function to encode and write a single chunk of file
 * 
 * @param code_table table of encodings
 * @param file_chunk chunk of file to encode
 * @param id id of the thread, used for writing
 * @param output_file output filestream to write to
 * @param m mutex to use
 * @param cv condition variable to use
 * @return encoding_results 
 */
encoding_results encode_chunk(vector<std::pair<int,int>> &code_table, vector<char> &file_chunk, int id, std::ofstream &output_file,
                                std::mutex &m, std::condition_variable &cv){
    Timer timer;
    timer.start("encode");
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
    for(auto &c : file_chunk){
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
    long time = timer.stop();
    encoding_results res;
    res.encode_time = time;
    char padding = max_size-buf_len;
    int chunk_size = buffer_vec.size();

    if(output_file.is_open()){
        // wait until the chunk of the thread can be written
        std::unique_lock lk(m);
        while(id != write_id){
            cv.wait(lk);
        }

        timer.start("write");
        // write size of chunk
        output_file.write(reinterpret_cast<const char *>(&chunk_size), sizeof(chunk_size));
        // write number of padding bits
        output_file.write(&padding, 1);
        // write the encoded binary
        output_file.write(buffer_vec.data(), buffer_vec.size());
        time = timer.stop();

        res.write_time = time;
        write_id++;
        cv.notify_all();
        return res;
    }
    else{
        return res;
    }
}

int main(int argc, char* argv[]){

    std::mutex m;
    std::condition_variable cv;

    string filename = "";
    string output_filename = "";
    int n_threads = 4;
    bool verbose = false;
    string log_folder = "";

    // used to log parallel frequency count without including time spent reading from file
    bool debug = false;

    // parse command line arguments
    int opt;

    while ((opt = getopt(argc, argv, "hi:o:t:vl:d")) != -1) {
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
        case 't':
            n_threads = atoi(optarg);
            break;
        case 'v':
            verbose = true;
            break;
        case 'l':
            log_folder = optarg;
            break;
        case 'd':
            debug = true;
            break;
        default:
            print_help();
            return 0;
        }
    }

    // fail if debug was enabled without logging enabled as well
    if(debug && log_folder == ""){
        cout << "Debug mode cannot be enabled without logging enabled." << endl;
        print_help();
        return 0;
    }
    // fail if input file does not exist or is missing
    if((filename == "" || !std::filesystem::exists(filename))){
        cout << "Input filename is missing or file does not exist." << endl;
        print_help();
        return 0;
    }
    // fail if no output file was given without debug enabled
    if((output_filename == "" && !debug)){
        cout << "Output filename is required." << endl;
        print_help();
        return 0;
    }

    // build path to save logs
    // assumes input file ends in 3 letter long file format e.g. .txt
    string log_file = "./" + log_folder + "/par/" + std::to_string(n_threads) + "_" + filename;
    log_file = log_file.substr(0, log_file.find_last_of('.'))+".csv";

    Logger logger(log_file, n_threads);
    Timer timer;
    long elapsed_time; 
    Timer tot_timer;

    // loop n_times
    tot_timer.start("total");
    write_id=0;
    std::ifstream file(filename);

    std::stringstream file_buffer;

    int filesize = std::filesystem::file_size(filename);

    // vector of buffers to store the file in chunks
    vector<vector<char>> file_chunks(n_threads);
    int chunk_size = filesize / n_threads;

    // vector of vectors storing partial character counts
    vector<vector<int>> partial_counts(n_threads, vector<int>(128, 0));

    // vector storing thread ids
    vector<std::future<long>> count_tids;

    // function acting as body of thread to count characters of a chunk of file
    auto count_chars = [&partial_counts, &file_chunks](int tid){
        Timer timer;
        timer.start("freq_time");
        for(int i=0; i<file_chunks[tid].size(); i++){
            partial_counts[tid][file_chunks[tid][i]]++;
        }
        long elapsed = timer.stop();
        return elapsed;
    };

    // time to read file, including the resizing of the buffer
    long read_time = 0;
    long freq_thread_overhead = 0;
    // read file
    logger.start("read_and_count");
        for(int i=0; i<n_threads; i++){
            timer.start("reading_input");
            if(i!=n_threads-1){
                file_chunks[i].resize(chunk_size);
                file.read(&(file_chunks[i])[0], chunk_size);
            }
            else{
                file_chunks[i].resize(chunk_size + filesize % n_threads);
                file.read(&(file_chunks[i])[0], chunk_size + filesize % n_threads);
            }
            read_time += timer.stop();

            if(!debug){
                timer.start("freq_thread_overhead");
                count_tids.push_back(move(std::async(std::launch::async, count_chars, i)));
                freq_thread_overhead += timer.stop();
            }
        }
        file.close();

        long par_freq_time = 0;
        if(debug){
            timer.start("par_freq_time");
            for(int i=0; i<n_threads; i++){
                count_tids.push_back(move(std::async(std::launch::async, count_chars, i)));
            }
        }

        long freq_time = 0;
        for(auto &t : count_tids){
            freq_time += t.get();
            
            // adding this line makes the threads faster for some reason
            // cout<<endl;
        }

        if(debug){
            par_freq_time = timer.stop();
            logger.add_stat("freq_time", par_freq_time);
        }

        // use array to store character counts
        // can be directly indexed using ASCII characters
        vector<int> count_vector(128, 0);

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
    if(!debug){logger.add_stat("freq_time", freq_time);}
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

        vector<std::future<encoding_results>> encode_tids;
        
        timer.start("write");
        std::ofstream output_file(output_filename, std::ios::binary);  
        
        if(output_file.is_open()){
            // write number of chunks          
            output_file.write(reinterpret_cast<const char *>(&n_threads), sizeof(n_threads));
            // write encoding table
            // more efficient ways exist
            for(auto &f : count_vector){
                output_file.write(reinterpret_cast<const char *>(&f), sizeof(f));
            }
            write_time += timer.stop();
        }

        timer.start("encode_thread_overhead");
        for(int i=0; i<n_threads; i++){
            encode_tids.push_back(move(std::async(std::launch::async, encode_chunk,
                                            std::ref(code_table), std::ref(file_chunks[i]), i, std::ref(output_file),
                                            std::ref(m), std::ref(cv))));
        }
        long encode_thread_overhead = timer.stop();

        for(int i=0; i<n_threads; i++){
            auto res = encode_tids[i].get();
            encode_time += res.encode_time;
            write_time += res.write_time;
        }
    elapsed_time = logger.stop();
    logger.add_stat("write", write_time);
    if(!debug){logger.add_stat("encode", encode_time);}
    else{logger.add_stat("encode", elapsed_time);}
    logger.add_stat("encode_thread_overhead", encode_thread_overhead);

    if(verbose){
        cout << "Encoding and writing the file took " << elapsed_time << " usecs." << endl;
        cout << "Encoding the file took " << encode_time << " usecs." << endl;
        cout << "Writing encoded file took " << write_time << " usecs." << endl;
    }

    logger.add_stat("total", tot_timer.stop());
    if(log_folder != ""){
        std::filesystem::create_directory("./" + log_folder);
        std::filesystem::create_directory("./" + log_folder + "/par");
        logger.write_logs(log_file);
    }
    return 0;
}