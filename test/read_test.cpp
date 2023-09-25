#include <iostream>
#include <functional>
#include <vector>
#include <algorithm>
#include <map>
#include <string>
#include <execution>
#include <thread>
#include <future>
#include <fstream>
#include "utimer.hpp"
#include "utils.hpp"

using std::cout, std::clog, std::endl, std::thread, std::vector, std::function, std::async, std::string;


int current_threads = 0;
std::mutex t_mutex;



int main(int argc, char* argv[]){

    // read filename from argument
    string filename = argv[1];

    // filestream variable file
    std::fstream file;
    // variable to store characters
    char ch;
 
    // TEST USING MAP
 
    // opening file
    file.open(filename.c_str());

    // store character counts in a map
    std::map<char, int> count_map;
    long u_map;
    {
        utimer t0("map", &u_map);
        while (file >> std::noskipws >> ch)
        {
            if(count_map.contains(ch)){
                count_map[ch]++;
            }
            else{
                count_map[ch] = 1;
            }
        }
    }

    file.close();

    // print character counts
    print_vector_pair(count_map);

    cout << "Execution w/ map took: " << u_map << " usecs;" << endl;

    // TEST USING ARRAY

    // opening file
    file.open(filename.c_str());

    // use array to store character counts
    // can be directly indexed using ASCII characters
    int count_array[128] = {0}; 

    long u_array;
    {
        utimer t0("array", &u_array);
        while (file >> std::noskipws >> ch)
        {
            count_array[int(ch)]++;
        }
    }
    file.close();

    clog << 10 << ", " << "LF" << ", " << count_array[10] << endl;
    clog << 13 << ", " << "CR" << ", " << count_array[13] << endl;

    // print character counts
    for(int i=32; i<127; i++){
        clog << i << ", " << char(i) << ", " << count_array[i] << endl;
    }

    cout << "Execution w/ array took: " << u_array << " usecs;" << endl;

    // TEST FOR VECTOR

    // opening file
    file.open(filename.c_str());

    // use array to store character counts
    // can be directly indexed using ASCII characters
    vector<int> count_vector(128);

    long u_vector;
    {
        utimer t0("array", &u_vector);
        while (file >> std::noskipws >> ch)
        {
            count_vector[int(ch)]++;
        }
    }
    file.close();

    clog << 10 << ", " << "LF" << ", " << count_vector[10] << endl;
    clog << 13 << ", " << "CR" << ", " << count_vector[13] << endl;

    // print character counts
    for(int i=32; i<127; i++){
        clog << i << ", " << char(i) << ", " << count_vector[i] << endl;
    }

    cout << "Execution w/ vector took: " << u_vector << " usecs;" << endl;


}