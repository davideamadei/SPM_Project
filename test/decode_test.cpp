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
    std::vector<int> count_vector(128);
    input_file.read(reinterpret_cast<char *>(&n_chunks), sizeof(n_chunks));

    for(int i=0;i<128;i++){
        input_file.read(reinterpret_cast<char *>(&(count_vector[i])), sizeof(count_vector[i]));
    }

    HuffmanTree ht = HuffmanTree(count_vector);
    std::ofstream output_file(output_filename, std::ios::binary);

    for(int i=0; i<n_chunks; i++){

        int chunk_size;
        char padding;

        input_file.read(reinterpret_cast<char *>(&chunk_size), sizeof(chunk_size));

        input_file.read(&padding, sizeof(padding));

        int real_size = chunk_size*8 - padding;

        std::vector<char> buffer_vec(chunk_size);
        for(auto &buf : buffer_vec){
            input_file.read(&buf, sizeof(buf));
        }      

        char ch;
        int buf_shift = 7;
        auto current_node = ht.getRoot();
        int maxsize = sizeof(char) *8;
        int counter = 0;
        int branch;
        for(auto &buf : buffer_vec){
            buf_shift = 7; 
            for(int j=0; j<maxsize; j++){
                if(current_node->getLeftChild()==nullptr){
                    ch = current_node->getCh();
                    output_file.write(&ch, 1);
                    current_node = ht.getRoot();
                }
                if(counter == real_size){
                    return 0;
                }
                if(((buf >> buf_shift) & 1)){
                    current_node = current_node->getRightChild();
                }
                else{
                    current_node = current_node->getLeftChild();
                }
                buf_shift--;
                counter++;
            }
        }
        
    }

}
