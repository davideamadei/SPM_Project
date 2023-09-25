#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <memory>
#include <bitset>
#include <bits/stdc++.h>

#include "utimer.hpp"
#include "utils.hpp"
#include "huffman_node_class.hpp"

using std::cout, std::clog, std::endl, std::thread, std::vector, std::function, std::async, std::string;


int current_threads = 0;
std::mutex t_mutex;


std::shared_ptr<HuffmanNode> min_node(std::queue<std::shared_ptr<HuffmanNode>> * q1, std::queue<std::shared_ptr<HuffmanNode>> * q2){
    if(q1->empty()){
        auto min = (q2->front());
        q2->pop();
        return min;
    }

    if(q2->empty()){
        auto min = (q1->front());
        q1->pop();
        return min;
    }

    if(q1->front()->getFrequency() < q2->front()->getFrequency()){
        auto min = (q1->front());
        q1->pop();
        return min;
    }
    else{
        auto min = (q2->front());
        q2->pop();
        return min;
    }
}

std::shared_ptr<HuffmanNode> createHuffmanTree(std::queue<std::shared_ptr<HuffmanNode>> * leaf_queue){
    std::queue<std::shared_ptr<HuffmanNode>> comp_queue;
    std::shared_ptr<HuffmanNode> min = nullptr;
    std::shared_ptr<HuffmanNode> second_min = nullptr;
    while(leaf_queue->size() + comp_queue.size() > 1){

        min = min_node(leaf_queue, &comp_queue);
        second_min = min_node(leaf_queue, &comp_queue);

        clog << min->getFrequency() << endl;
        clog << second_min->getFrequency() << endl;
        std::shared_ptr<HuffmanNode> new_node(new HuffmanNode(0, min->getFrequency()+second_min->getFrequency()));

        new_node->setLeftChild(min);
        new_node->setRightChild(second_min);
        // min->setParent(new_node);
        // second_min->setParent(new_node);

        comp_queue.push(new_node);
    }

    if(comp_queue.empty()){
        return leaf_queue->front();
    }
    else{
        return comp_queue.front();
    }
}

void extract_codes_rec(std::shared_ptr<HuffmanNode> huffman_tree, string current_code, std::shared_ptr<vector<string>> code_table){

    // base case, a node has no children
    if(huffman_tree->getLeftChild()==nullptr && huffman_tree->getRightChild()==nullptr){
        (*code_table)[int(huffman_tree->getCh())] = current_code;
        return;
    }

    string left_code = current_code+"0";
    string right_code = current_code+"1";

    // bitshift to left the given code and add 1 for the right child
    // int left_code = (current_code << 1);
    // int right_code = (current_code << 1) + 1;
    
    extract_codes_rec(huffman_tree->getLeftChild(), left_code, code_table);
    extract_codes_rec(huffman_tree->getRightChild(), right_code, code_table);
    
    return;
}

std::shared_ptr<vector<string>> extract_codes(std::shared_ptr<HuffmanNode> huffman_tree){
    std::shared_ptr<vector<string>>code_table(new vector<string>(128, "0"));

    string left_code = "0";
    string right_code = "1";
    
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

    vector<int> count_vector;
    long u_vector;
    {
        utimer t0("array", &u_vector);
        
        count_vector = read_file(filename);

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
    vector<std::shared_ptr<HuffmanNode>> ht_leaves;
    for(int i=0; i<128; i++){
        if(count_vector[i] != 0){
            auto tmp = std::shared_ptr<HuffmanNode>(new HuffmanNode(i, count_vector[i]));
            ht_leaves.push_back(tmp);
        }
    }

    auto compare_node = [](std::shared_ptr<HuffmanNode> n1, std::shared_ptr<HuffmanNode> n2) 
                        {return n1->getFrequency() < n2->getFrequency();};
    sort(ht_leaves.begin(), ht_leaves.end(), compare_node);

    std::queue<std::shared_ptr<HuffmanNode>> leaf_queue;
    for(auto &e : ht_leaves){
        leaf_queue.push(e);
    }

    std::shared_ptr<HuffmanNode> root = createHuffmanTree(&leaf_queue);
    std::shared_ptr<vector<string>>code_table = extract_codes(root);

    // for(int i = 0; i<code_table->size(); i++){
    //     cout<<char(i) << ", " << count_vector[i] << ", " << std::bitset<32>((*code_table)[i]) << endl;
    // }
    for(int i = 0; i<code_table->size(); i++){
        clog<<char(i) << ", " << count_vector[i] << ", " << (*code_table)[i] << endl;
    }
    // }
    // cout<<"Tree creation and code extraction took " << tree_timer << " usecs" << endl;

    std::ifstream file(filename);

    std::stringstream file_buffer;
    file_buffer << file.rdbuf();
    string encoded_string = file_buffer.str();
    
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