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

    // for(int i = 0; i<code_table->size(); i++){
    //     cout<<char(i) << ", " << count_vector[i] << ", " << std::bitset<32>((*code_table)[i]) << endl;
    // }
    for(int i = 0; i<code_table.size(); i++){
        clog<<char(i) << ", " << count_vector[i] << ", ";
        auto code = code_table[i];
        for(int j=0; j<code.size(); j++){
            clog << code[j];
        }
        clog<<endl;
    }
    // }
    // cout<<"Tree creation and code extraction took " << tree_timer << " usecs" << endl;


    string encoded_string = "01010100010";


    cout<< "File is " << encoded_string.size() << " characters long" <<endl;


    std::ofstream encoded_file("encoded_"+filename, std::ios::binary);

    char buffer = 0;
    int i = 0;
    for(int j=0; j<encoded_string.size(); j++){
        if(i==8){
            encoded_file.write(&buffer, 1);
            buffer = 0;
            i = 0;
        }
        char bit = encoded_string[j];
        if(bit == '0'){
            buffer = buffer << 1; 
        }
        else{
            buffer = (buffer << 1) + 1; 
        }
        i++;
    }
    if(i!=0){
        buffer = buffer << (8-i);
        encoded_file.write(&buffer, 1);
    }
}