/**
 * @file decode_test.cpp
 * @author Davide Amadei (davide.amadei97@gmail.com)
 * @brief file containing code to decode a file encoded with huffman
 * @date 2023-10-04
 * 
 * Rough implementation, only for testing purposes. Can be very slow with larger files 
 */
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <memory>
#include <bitset>
#include <bits/stdc++.h>

#include "huffman_tree.hpp"


using std::cout, std::clog, std::endl, std::string;

int main(int argc, char* argv[]){
    string filename = argv[1];
    string output_filename = argv[2];

    std::ifstream input_file(filename, std::ios::binary);
    int n_chunks;
    std::vector<int> count_vector(256);
    input_file.read(reinterpret_cast<char *>(&n_chunks), sizeof(n_chunks));
    for(int i=0;i<256;i++){
        input_file.read(reinterpret_cast<char *>(&(count_vector[i])), sizeof(count_vector[i]));
    }

    HuffmanTree ht = HuffmanTree(count_vector);
    std::ofstream output_file(output_filename, std::ios::binary);


    for(int i=0; i<n_chunks; i++){

        int chunk_size;
        char padding;
        input_file.read(reinterpret_cast<char *>(&chunk_size), sizeof(chunk_size));

        input_file.read(&padding, sizeof(padding));
        int bitsize = chunk_size*8 - padding;

        std::vector<char> buffer_vec(chunk_size);
        for(auto &buf : buffer_vec){
            input_file.read(&buf, sizeof(buf));
        }      

        unsigned char ch;
        auto current_node = ht.getRoot();
        int maxsize = sizeof(char) *8;
        int counter = 0;
        int branch;

        for(auto &buf : buffer_vec){
            for(int buf_shift=7; buf_shift>=0; buf_shift--){
                if(current_node->getLeftChild()==nullptr){
                    ch = current_node->getCh();
                    output_file.write(reinterpret_cast<char *>(&ch), 1);
                    current_node = ht.getRoot();
                }
                if(counter == bitsize){
                    continue;
                }
                if(((buf >> buf_shift) & 1)){
                    current_node = current_node->getRightChild();
                }
                else{
                    current_node = current_node->getLeftChild();
                }
                counter++;
            }
        }
        if(current_node->getLeftChild()==nullptr){
            ch = current_node->getCh();
            output_file.write(reinterpret_cast<char *>(&ch), 1);
            current_node = ht.getRoot();
        }
        
    }
}
