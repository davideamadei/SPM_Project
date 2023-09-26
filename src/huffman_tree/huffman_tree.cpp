#include "huffman_tree.hpp"
#include <queue>
#include <iostream>
HuffmanNode::HuffmanNode(char init_ch, int init_frequency){
        ch = init_ch;
        frequency = init_frequency;
    }

char HuffmanNode::getCh(){return ch;}
int HuffmanNode::getFrequency(){return frequency;}
std::shared_ptr<HuffmanNode> HuffmanNode::getLeftChild(){return left_child;}
std::shared_ptr<HuffmanNode> HuffmanNode::getRightChild(){return right_child;}
std::shared_ptr<HuffmanNode> HuffmanNode::getParent(){return parent;}

bool HuffmanNode::setLeftChild(std::shared_ptr<HuffmanNode> new_child){
    // child can be inserted only if it was previously nullptr
    // returns true if succesful, false otherwise
    if(left_child==nullptr){
        left_child = new_child;
        return true;
    }
    else{
        return false;
    }
}
bool HuffmanNode::setRightChild(std::shared_ptr<HuffmanNode> new_child){
    // child can be inserted only if it was previously nullptr
    // returns true if succesful, false otherwise
    if(right_child==nullptr){
        right_child = new_child;
        return true;
    }
    else{
        return false;
    }
}
bool HuffmanNode::setParent(std::shared_ptr<HuffmanNode> new_parent){
    // parent can be inserted only if it was previously nullptr
    // returns true if succesful, false otherwise
    if(parent==nullptr){
        parent = new_parent;
        return true;
    }
    else{
        return false;
    }
}

void HuffmanTree::extract_codes_rec(std::shared_ptr<HuffmanNode> huffman_tree, std::vector<bool> *code){

    // base case, a node has no children
    if(huffman_tree->getLeftChild()==nullptr && huffman_tree->getRightChild()==nullptr){
        code_table[int(huffman_tree->getCh())] = std::vector<bool>(*code);
        return;
    }


    code->push_back(0);
    extract_codes_rec(huffman_tree->getLeftChild(), code);
    code->pop_back();
    code->push_back(1);    
    extract_codes_rec(huffman_tree->getRightChild(), code);
    code->pop_back();

    for(int i=0;i<code_table[50].size();i++){
            std::cout<<code_table[50][i];
        }
        std::cout<<std::endl;
        std::cout<<std::endl;
        
    return;
}

HuffmanTree::HuffmanTree(std::vector<int> char_counts){
    std::vector<std::shared_ptr<HuffmanNode>> ht_leaves;
    for(int i=0; i<128; i++){
        if(char_counts[i] != 0){
            auto tmp = std::shared_ptr<HuffmanNode>(new HuffmanNode(i, char_counts[i]));
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

    std::queue<std::shared_ptr<HuffmanNode>> comp_queue;
    std::shared_ptr<HuffmanNode> min = nullptr;
    std::shared_ptr<HuffmanNode> second_min = nullptr;


    auto min_node = [&] (){
        if(leaf_queue.empty()){
            auto min = (comp_queue.front());
            comp_queue.pop();
            return min;
        }

        if(comp_queue.empty()){
            auto min = (leaf_queue.front());
            leaf_queue.pop();
            return min;
        }

        auto leaf = leaf_queue.front();
        auto comp = comp_queue.front();

        if(leaf ->getFrequency() < comp->getFrequency()){
            auto min = leaf;
            leaf_queue.pop();
            return min;
        }
        else{
            auto min = comp;
            comp_queue.pop();
            return min;
        }
    };

    while(leaf_queue.size() + comp_queue.size() > 1){

        min = min_node();
        second_min = min_node();

        std::clog << min->getFrequency() << std::endl;
        std::clog << second_min->getFrequency() << std::endl;
        std::shared_ptr<HuffmanNode> new_node(new HuffmanNode(0, min->getFrequency()+second_min->getFrequency()));

        new_node->setLeftChild(min);
        new_node->setRightChild(second_min);
        // min->setParent(new_node);
        // second_min->setParent(new_node);

        comp_queue.push(new_node);
    }

    if(comp_queue.empty()){
        root = leaf_queue.front();
    }
    else{
        root = comp_queue.front();
    }

    std::vector<bool> code;
    code.push_back(0);
    
    extract_codes_rec(root->getLeftChild(), &code);

    code.pop_back();    
    code.push_back(1);
    extract_codes_rec(root->getRightChild(), &code);
}
std::shared_ptr<HuffmanNode> HuffmanTree::getRoot(){return root;}
std::vector<std::vector<bool>> HuffmanTree::getCodes(){return code_table;}