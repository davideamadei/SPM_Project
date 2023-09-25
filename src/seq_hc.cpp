#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <bits/stdc++.h>

#include "utimer.hpp"
#include "utils.hpp"
#include "huffman_node_class.hpp"

using std::cout, std::clog, std::endl, std::thread, std::vector, std::function, std::async, std::string;


int current_threads = 0;
std::mutex t_mutex;


HuffmanNode* min_node(std::queue<HuffmanNode*> * q1, std::queue<HuffmanNode*> * q2){
    if(q1->empty()){
        auto min = q2->front();
        q2->pop();
        return min;
    }

    if(q2->empty()){
        auto min = q1->front();
        q1->pop();
        return min;
    }

    if(q1->front() ->getFrequency() < q2->front() -> getFrequency()){
        auto min = q1->front();
        q1->pop();
        return min;
    }
    else{
        auto min = q2->front();
        q2->pop();
        return min;
    }
}

HuffmanNode* createHuffmanTree(std::queue<HuffmanNode*> leaf_queue){
    std::queue<HuffmanNode*> comp_queue;
    HuffmanNode* min = nullptr;
    HuffmanNode* second_min = nullptr;
    while(leaf_queue.size() + comp_queue.size() > 1){

        min = min_node(&leaf_queue, &comp_queue);
        second_min = min_node(&leaf_queue, &comp_queue);

        clog << min->getFrequency() << endl;
        clog << second_min->getFrequency() << endl;
        HuffmanNode* new_node = new HuffmanNode(0, min->getFrequency()+second_min->getFrequency());
        new_node->setLeftChild(min);
        new_node->setRightChild(second_min);
        min->setParent(new_node);
        min->setParent(new_node);

        comp_queue.push(new_node);
    }
    if(comp_queue.empty()){
        return leaf_queue.front();
    }
    else{
        return comp_queue.front();
    }
}
// vector<int> extract_code_table(tree);

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
    long timer;
    {
        utimer t0("test", &timer);
    vector<HuffmanNode*> ht_leaves;
    for(int i=0; i<128; i++){
        if(count_vector[i] != 0){
            auto tmp = new HuffmanNode(i, count_vector[i]);
            ht_leaves.push_back(tmp);
        }
    }

    auto compare_node = [](HuffmanNode* n1, HuffmanNode* n2) {return n1->getFrequency() < n2->getFrequency();};
    sort(ht_leaves.begin(), ht_leaves.end(), compare_node);

    std::queue<HuffmanNode*> leaf_queue;
    for(auto &e : ht_leaves){
        leaf_queue.push(e);
    }

    HuffmanNode* root = createHuffmanTree(leaf_queue);

    }
    cout << timer << " usecs" << endl;
}