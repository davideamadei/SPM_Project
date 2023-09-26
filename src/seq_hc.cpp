#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <memory>
#include <bitset>
#include <bits/stdc++.h>

#include "utimer.hpp"
#include "utils.hpp"
#include "huffman_tree.hpp"

using std::cout, std::clog, std::endl, std::string;

int main(int argc, char* argv[]){

    // read filename from argument
    string filename = argv[1];
    string output_filename = argv[2];

    std::ifstream file(filename);

    std::stringstream file_buffer;
    file_buffer << file.rdbuf();
    string file_str = file_buffer.str();

    std::vector<int> count_vector;

    count_vector = count_characters(file_str);

    for(int i=0;i<file_str.size();i++){
        if(file_str[i]<0){
            cout<<int(file_str[i])<<endl;
            }
    }
    clog << 10 << ", " << "LF" << ", " << count_vector[10] << endl;
    clog << 13 << ", " << "CR" << ", " << count_vector[13] << endl;

    // print character counts
    for(int i=32; i<127; i++){
        clog << i << ", " << char(i) << ", " << count_vector[i] << endl;
    }

    HuffmanTree ht(count_vector);

    auto code_table = ht.getCodes();


    std::vector<char> buffer_vec;
    int buf_len = 0;
    char buffer = 0;
    int remaining;
    int length;
    int code;
    int max_size = sizeof(char) * 8;

    //there for consistency with parallel version
    int n_chunks = 1;

    for(int i=0; i<file_str.size(); i++){
        auto code_pair = code_table[file_str[i]];
        length = code_pair.first;
        code = code_pair.second;
        remaining = length;
        while(remaining != 0){
            if(buf_len==max_size){
                buffer_vec.push_back(buffer);
                buffer = 0;
                buf_len = 0;
            }
            if(buf_len+remaining <= max_size){
                buffer = (buffer << remaining) | code;
                buf_len += remaining;
                remaining = 0;
            }
            // i + remaining >= max_size
            else{
                int shift = max_size-buf_len;
                buffer = (buffer << shift) | (code >> (remaining - shift));
                remaining -= shift;
                buf_len = max_size;
            }
        }
        
    }

    if(buf_len!=0){
        buffer_vec.push_back(buffer << (max_size-buf_len));
        // buffer = buffer << (8-i);
        // output_file.write(&buffer, 1);
    }

    char ending_padding = max_size - buf_len;

    int chunk_byte_size = buffer_vec.size();

    cout << "File is " << file_str.size() << " characters long" <<endl;
    cout << "Encoded file is " << chunk_byte_size << " bits" << endl;

    std::ofstream output_file(output_filename, std::ios::binary);


    output_file.write(reinterpret_cast<const char *>(&n_chunks), sizeof(n_chunks));

    for(auto &f : count_vector){
        output_file.write(reinterpret_cast<const char *>(&f), sizeof(f));
    }

    output_file.write(reinterpret_cast<const char *>(&chunk_byte_size), sizeof(chunk_byte_size));
    output_file.write(&ending_padding, 1);
    output_file.write(buffer_vec.data(), buffer_vec.size());
    return 0;
}