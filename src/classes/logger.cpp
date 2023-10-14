/**
 * @file logger.cpp
 * @author Davide Amadei (davide.amadei97@gmail.com)
 * @brief file containing the implementation of the logging classes
 * @date 2023-10-04
 * 
 */
#include "logger.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>


// Timer 

/**
 * @brief method to start the timer
 * 
 * @param msg string to identify the currently started timer
 */
void Timer::start(std::string msg){
    start_time = std::chrono::system_clock::now();
    timer_str = msg;
    return;
}

/**
 * @brief method to stop the timer
 * 
 * @return long number of microseconds elapsed
 */
long Timer::stop(){
    stop_time = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = stop_time - start_time;
    auto elapsed_usec = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    return elapsed_usec;
}

/**
 * @brief get the string of the timer
 * 
 * @return std::string the string of the timer
 */
std::string Timer::getStr(){return timer_str;}



// Logger

/**
 * @brief Construct a new Logger:: Logger object
 * 
 * If filename is empty the logger is initialized from scratch
 * 
 * @param filename file to use initialize the logger
 * @param n_threads number of threads that the program is running
 */
Logger::Logger(std::string filename, int n_threads){
    if(!std::filesystem::exists(filename)){
        cumulative_stats["stats"] = n_threads;
        cumulative_stats["times_run"] = 1;
        return;
    }
    std::ifstream file(filename);
    std::string word;
    std::string stat;
    int i = 0;

    // read stats row
    file >> stat;
    file >> word;
    cumulative_stats[stat] = std::stod(word);
    // read times_run row
    file >> stat;
    file >> word;
    auto n_times = std::stod(word);
    cumulative_stats[stat] = stod(word) + 1;

    while(file>>word)
    {
        if(i%2 == 0){
            stat = word;
        }
        else{
            cumulative_stats[stat] = std::stod(word);
        }
        i++;
    }
}

/**
 * @brief method to start the internal timer of the logger
 * 
 * @param stat_name 
 */
void Logger::start(std::string stat_name){
    timer.start(stat_name);
    return;
}

/**
 * @brief method to stop the currently running timer of the log
 * 
 * Once stopped it saves the time in an internal map. If the stat already existed the time is summed up the currently present value
 * 
 * @return long number of microseconds elapsed
 */
long Logger::stop(){
    auto elapsed = timer.stop();
    std::string stat_name = timer.getStr();
    if(cumulative_stats.contains(stat_name)){
        cumulative_stats[stat_name] += (elapsed - cumulative_stats[stat_name])/cumulative_stats["times_run"];
    }
    else{
        cumulative_stats[stat_name] = elapsed;
    }

    return elapsed;
}

/**
 * @brief method to add a stat manually to the logger
 * 
 * @param stat_name name of the stat to add
 * @param time amount of time for the stat to be inserted
 */
void Logger::add_stat(std::string stat_name, long time){
    if(cumulative_stats.contains(stat_name)){
        cumulative_stats[stat_name] += (time - cumulative_stats[stat_name])/cumulative_stats["times_run"];
    }
    else{
        cumulative_stats[stat_name] = time;
    }
    return;
}

/**
 * @brief function to write logs to file
 * 
 * @param filename file in which to save the logs (will be ovewritten with each write)
 * @param n_tries number of times the code was executed. Needed to average the gathered times
 */
void Logger::write_logs(std::string filename){
    std::ofstream output_file(filename, std::ios::binary);
    double n_times = cumulative_stats["times_run"];
    output_file << "stats " << cumulative_stats["stats"] << std::endl;
    output_file << "times_run " << cumulative_stats["times_run"] << std::endl;
    for(auto &s : cumulative_stats){
        if(s.first != "stats" && s.first != "times_run"){
            output_file << s.first << " " << s.second << std::endl;
        }
    }
    return;
}