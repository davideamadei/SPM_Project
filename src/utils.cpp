#include "utils.hpp"
#include <fstream>
#include <string>
#include "utimer.hpp"

std::vector<int> read_file(std::string filename){
    // filestream variable file
    std::fstream file;
    // variable to store characters
    char ch;

    // opening file
    file.open(filename.c_str());

    // use array to store character counts
    // can be directly indexed using ASCII characters
    std::vector<int> count_vector(128);

    long u_vector;
    {
        utimer t0("array", &u_vector);
        while (file >> std::noskipws >> ch)
        {
            count_vector[int(ch)]++;
        }
    }
    file.close();
    return count_vector;
}