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

using std::cout, std::clog, std::endl, std::thread, std::vector, std::function, std::async, std::string;


int current_threads = 0;
std::mutex t_mutex;


void extract_codes_rec(std::shared_ptr<HuffmanNode> huffman_tree, vector<bool> current_code,
                        std::shared_ptr<vector<vector<bool>>> code_table){

    // base case, a node has no children
    if(huffman_tree->getLeftChild()==nullptr && huffman_tree->getRightChild()==nullptr){
        (*code_table)[huffman_tree->getCh()] = current_code;
        return;
    }

    vector<bool> left_code = current_code;
    left_code.push_back(0);
    vector<bool> right_code = current_code;
    right_code.push_back(1);

    // bitshift to left the given code and add 1 for the right child
    // int left_code = (current_code << 1);
    // int right_code = (current_code << 1) + 1;
    
    extract_codes_rec(huffman_tree->getLeftChild(), left_code, code_table);
    extract_codes_rec(huffman_tree->getRightChild(), right_code, code_table);
    
    return;
}

std::shared_ptr<vector<vector<bool>>> extract_codes(std::shared_ptr<HuffmanNode> huffman_tree){
    std::shared_ptr<vector<vector<bool>>>code_table(new vector<vector<bool>>(128));

    vector<bool> left_code;
    left_code.push_back(0);
    vector<bool> right_code;
    right_code.push_back(1);
    
    extract_codes_rec(huffman_tree->getLeftChild(), left_code, code_table);
    extract_codes_rec(huffman_tree->getRightChild(), right_code, code_table);
    
    return code_table;
}

int size_tree(std::shared_ptr<HuffmanNode> htree){
    if(htree==nullptr){return 0;}
    return size_tree(htree->getLeftChild())+1+size_tree(htree->getRightChild());
}


int main(int argc, char* argv[]){

    // read filename from argument
    string filename = argv[1];

    std::ifstream file(filename);

    std::stringstream file_buffer;
    file_buffer << file.rdbuf();
    string file_str = file_buffer.str();

    vector<int> count_vector;
    long u_vector;
    {
        utimer t0("array", &u_vector);
        
        count_vector = count_characters(file_str);

    }

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

    cout << "Reading file took: " << u_vector << " usecs;" << endl;

    // long tree_timer;
    // {
    //     utimer t0("tree_code", &tree_timer);
    

    HuffmanTree ht(count_vector);

    auto code_table = ht.getCodes();

    vector<char> buffer_vec;
    int buf_len = 0;
    char buffer = 0;
    int remaining;
    int length;
    int code;
    int max_size = sizeof(char) * 8;
    for(int i=0; i<file_str.size(); i++){
        // cout<<i << " " << file_str[i]<<endl;
        auto code_pair = code_table[file_str[i]];
        length = code_pair.first;
        code = code_pair.second;
        remaining = length;
        while(remaining != 0){
            if(buf_len==max_size){
                buffer_vec.push_back(buffer);
                // output_file.write(&buffer, 1);
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

    cout<< "File is " << file_str.size() << " characters long" <<endl;


    std::ofstream output_file("encoded_"+filename, std::ios::binary);


    long timer;
    {
        utimer to("test", &timer);
    
    output_file.write(buffer_vec.data(), buffer_vec.size());
    }
    cout<<timer<<endl;
}