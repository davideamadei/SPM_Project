#pragma once

#include <memory>
#include <vector>
#include <queue>
class HuffmanNode{
    private:
        char ch;
        int frequency;
        std::shared_ptr<HuffmanNode> left_child = nullptr;
        std::shared_ptr<HuffmanNode> right_child = nullptr;
        std::shared_ptr<HuffmanNode> parent = nullptr;
    public:
        HuffmanNode(char init_ch, int init_frequency);

        bool leaf = true;

        char getCh();
        int getFrequency();
        std::shared_ptr<HuffmanNode> getLeftChild();
        std::shared_ptr<HuffmanNode> getRightChild();
        std::shared_ptr<HuffmanNode> getParent();

        bool setLeftChild(std::shared_ptr<HuffmanNode> new_child);
        bool setRightChild(std::shared_ptr<HuffmanNode> new_child);
        bool setParent(std::shared_ptr<HuffmanNode> new_parent);
};


class HuffmanTree{
    private:
        std::shared_ptr<HuffmanNode> root;
        std::vector<std::vector<bool>> code_table = std::vector<std::vector<bool>>(128);
        void extract_codes_rec(std::shared_ptr<HuffmanNode> huffman_tree, std::vector<bool> *current_code);

    public:
        HuffmanTree(std::vector<int> char_counts);
        std::shared_ptr<HuffmanNode> getRoot();
        std::vector<std::vector<bool>> getCodes();

};