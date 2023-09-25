#pragma once

class HuffmanNode{
    private:
        char ch;
        int frequency;
        HuffmanNode* left_child = nullptr;
        HuffmanNode* right_child = nullptr;
        HuffmanNode* parent = nullptr;
    public:
        HuffmanNode(char init_ch, int init_frequency);

        char getCh();
        int getFrequency();
        HuffmanNode* getLeftChild();
        HuffmanNode* getRightChild();
        HuffmanNode* getParent();

        bool setLeftChild(HuffmanNode* new_child);
        bool setRightChild(HuffmanNode* new_child);
        bool setParent(HuffmanNode* new_parent);
};