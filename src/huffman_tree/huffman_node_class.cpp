#include "huffman_node_class.hpp"

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