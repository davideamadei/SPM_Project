/**
 * @file huffman_tree.hpp
 * @author Davide Amadei (davide.amadei97@gmail.com)
 * @brief header for the classes implementing the Huffman tree
 * @date 2023-10-04
 * 
 * 
 */
#pragma once

#include <memory>
#include <vector>
#include <queue>


/**
 * @brief class containing a single node of the Huffman tree
 * 
 */
class HuffmanNode{
    private:
        /**
         * @brief character contained in the node, only meaningful if node is a leaf
         * 
         */
        char ch;
        /**
         * @brief frequency of the character if node is a leaf, sum of the frequencies of the children subtrees otherwise
         * 
         */
        int frequency;
        /**
         * @brief pointer to left child
         * 
         */
        std::shared_ptr<HuffmanNode> left_child = nullptr;
        /**
         * @brief pointer to right child
         * 
         */
        std::shared_ptr<HuffmanNode> right_child = nullptr;
        /**
         * @brief pointer to parent node. Currently unused
         * 
         */
        std::shared_ptr<HuffmanNode> parent = nullptr;
    public:
        HuffmanNode(unsigned char init_ch, int init_frequency);

        unsigned char getCh();
        int getFrequency();
        std::shared_ptr<HuffmanNode> getLeftChild();
        std::shared_ptr<HuffmanNode> getRightChild();
        std::shared_ptr<HuffmanNode> getParent();

        bool setLeftChild(std::shared_ptr<HuffmanNode> new_child);
        bool setRightChild(std::shared_ptr<HuffmanNode> new_child);
        bool setParent(std::shared_ptr<HuffmanNode> new_parent);
};


/**
 * @brief class containing the Huffman tree and the table of encodings
 * 
 * Only the pointer to the root of the tree is actually stored in the object, the rest of the
 * tree can be accessed by using the pointers stored in each node.
 * The implementation assumes that the file being encoded is an ASCII file and will probably break otherwise.
 * 
 */
class HuffmanTree{
    private:
        /**
         * @brief pointer to the root of the tree
         * 
         */
        std::shared_ptr<HuffmanNode> root;
        /**
         * @brief vector containing the table of encodings
         * 
         * Stored as a pair of integers, the first is the amount of the relevant bits and the second is the actual code stored
         * as an integer.
         * 
         */
        std::vector<std::pair<int, int>> code_table = std::vector<std::pair<int, int>>(256);
        void extract_codes_rec(std::shared_ptr<HuffmanNode> huffman_tree, int code, int length);

    public:
        HuffmanTree(std::vector<int> char_counts);
        std::shared_ptr<HuffmanNode> getRoot();
        std::vector<std::pair<int, int>> getCodes();

};