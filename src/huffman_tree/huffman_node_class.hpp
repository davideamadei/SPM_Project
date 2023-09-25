#pragma once

#include <memory>
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