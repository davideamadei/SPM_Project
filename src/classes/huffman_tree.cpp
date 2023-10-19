/**
 * @file huffman_tree.cpp
 * @author Davide Amadei (davide.amadei97@gmail.com)
 * @brief file containing the implementation of the Huffman tree classes
 * @date 2023-10-04
 * 
 * 
 */
#include "huffman_tree.hpp"
#include <queue>
#include <iostream>

//Huffman node implementation

/**
 * @brief Construct a new Huffman Node:: Huffman Node object
 * 
 * @param init_ch character to store in the node
 * @param init_frequency frequency to store in the node
 */
HuffmanNode::HuffmanNode(unsigned char init_ch, int init_frequency){
        ch = init_ch;
        frequency = init_frequency;
    }

/**
 * @brief getter method for character stored
 * 
 * @return char 
 */
unsigned char HuffmanNode::getCh(){return ch;}
/**
 * @brief getter method for frequency stored
 * 
 * @return int 
 */
int HuffmanNode::getFrequency(){return frequency;}
/**
 * @brief getter method for pointer to left child
 * 
 * @return std::shared_ptr<HuffmanNode> 
 */
std::shared_ptr<HuffmanNode> HuffmanNode::getLeftChild(){return left_child;}
/**
 * @brief getter method for pointer to right child
 * 
 * @return std::shared_ptr<HuffmanNode> 
 */
std::shared_ptr<HuffmanNode> HuffmanNode::getRightChild(){return right_child;}
/**
 * @brief getter method for pointer to parent node
 * 
 * @return std::shared_ptr<HuffmanNode> 
 */
std::shared_ptr<HuffmanNode> HuffmanNode::getParent(){return parent;}

/**
 * @brief setter method for pointer to left child
 * 
 * @param new_child 
 * @return true if class member was previously unset
 * @return false otherwise
 */
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
/**
 * @brief setter method for pointer to right child
 * 
 * @param new_child 
 * @return true if class member was previously unset
 * @return false otherwise
 */
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
/**
 * @brief setter method for pointer to parent
 * 
 * @param new_parent
 * @return true if class member was previously unset
 * @return false otherwise
 */
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



// Huffman Tree implementation

/**
 * @brief getter method for the root of the tree
 * 
 * @return std::shared_ptr<HuffmanNode> 
 */
std::shared_ptr<HuffmanNode> HuffmanTree::getRoot(){return root;}
/**
 * @brief getter method for the table of encodings
 * 
 * @return std::vector<std::pair<int, int>> 
 */
std::vector<std::pair<int, int>> HuffmanTree::getCodes(){return code_table;}

/**
 * @brief helper recursive function to extract the table of encodings
 * 
 * At each call takes the code and bitshifts it left. To the left child it is passed as is, to the right child is increased by 1.
 * Length is increased by 1 in both cases, keeping track of the number of parent nodes traversed to reach the current node.
 * When a leaf is reached the code is saved inside the object containing the code table.
 * 
 * @param huffman_tree pointer to subtree currently being computed
 * @param code the current encoding
 * @param length the lenght (releavant bits) of the current encoding
 */
void HuffmanTree::extract_codes_rec(std::shared_ptr<HuffmanNode> huffman_tree, int code, int length){

    // base case, a node has no children
    if(huffman_tree->getLeftChild()==nullptr && huffman_tree->getRightChild()==nullptr){
        code_table[int(huffman_tree->getCh())] = std::make_pair(length, code);
        return;
    }

    // bitshift code to the left
    code<<=1;
    extract_codes_rec(huffman_tree->getLeftChild(), code, length+1);
    code+=1;    
    extract_codes_rec(huffman_tree->getRightChild(), code, length+1);
        
    return;
}


/**
 * @brief Construct a new Huffman Tree:: Huffman Tree object
 * 
 * Creates the huffman tree and extracts the corresponding table of encodings from it
 * 
 * @param char_counts vector containing the frequencies of the characters
 */
HuffmanTree::HuffmanTree(std::vector<int> char_counts){
    // vector containing the leaves of the tree
    std::vector<std::shared_ptr<HuffmanNode>> ht_leaves;
    for(int i=0; i<char_counts.size(); i++){
        if(char_counts[i] != 0){
            ht_leaves.push_back(std::shared_ptr<HuffmanNode>(new HuffmanNode(i, char_counts[i])));
        }
    }
    // sort the leaves by frequency, necessary for creating the huffman tree
    auto compare_node = [](std::shared_ptr<HuffmanNode> n1, std::shared_ptr<HuffmanNode> n2) 
        {return n1->getFrequency() < n2->getFrequency();};
    sort(ht_leaves.begin(), ht_leaves.end(), compare_node);

    // insert leaves in a queue
    std::queue<std::shared_ptr<HuffmanNode>> leaf_queue;
    for(auto &e : ht_leaves){
        leaf_queue.push(e);
    }

    std::queue<std::shared_ptr<HuffmanNode>> comp_queue;
    std::shared_ptr<HuffmanNode> min = nullptr;
    std::shared_ptr<HuffmanNode> second_min = nullptr;

    // function returning the minimum between the front element of two queues
    // minimum is popped before returning
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

    // loop to create the huffman tree
    while(leaf_queue.size() + comp_queue.size() > 1){

        min = min_node();
        second_min = min_node();

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

    //calls to the function extracting the table of encodings
    extract_codes_rec(root->getLeftChild(), 0, 1);
    extract_codes_rec(root->getRightChild(), 1, 1);
}