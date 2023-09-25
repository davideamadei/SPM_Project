#pragma once

#include <vector>
#include <map>
#include <string>
#include <iostream>

template <class T>
void print_vector(std::vector<T> vec){
    for (int i = 0; i < vec.size(); i++) {
        std::cout << vec[i] << " ";
    }
    std::clog << std::endl;
    return;
}


template <typename T1, typename T2>
void print_vector_pair(std::map<T1, T2> vec){
    for(auto &e : vec) {
        std::clog << e.first << "," << e.second << std::endl;
    }
    return;
}

std::vector<int> read_file(std::string filename);