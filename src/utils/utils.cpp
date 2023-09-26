#include "utils.hpp"
#include <fstream>
#include <string>
#include "utimer.hpp"

std::vector<int> count_characters(std::string file){
    // variable to store characters
    char ch;

    // use array to store character counts
    // can be directly indexed using ASCII characters
    std::vector<int> count_vector(128);

    long u_vector;
    {
        utimer t0("array", &u_vector);
        for(int i=0; i<file.size(); i++)
        {
            count_vector[file[i]]++;
        }
    }
    return count_vector;
}