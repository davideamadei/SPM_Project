/**
 * @file logger.cpp
 * @author Davide Amadei (davide.amadei97@gmail.com)
 * @brief file containing the implementation of the logging classes
 * @date 2023-10-04
 * 
 */
#include "logger.hpp"
#include <fstream>

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
        cumulative_stats[stat_name] += elapsed;
    }
    else{
        cumulative_stats[stat_name] = elapsed;
    }

    return elapsed;
}

/**
 * @brief function to write logs to file
 * 
 * @param filename file in which to save the logs (will be ovewritten with each write)
 * @param n_tries number of times the code was executed. Needed to average the gathered times
 */
void Logger::write_logs(std::string filename, int n_tries){
    std::ofstream output_file(filename, std::ios::binary);
    double avg;
    output_file << "stat" << "," << "usecs" << std::endl;
    for(auto &s : cumulative_stats){
        avg = s.second / n_tries;
        output_file << s.first << "," << avg << std::endl;
    }
    return;
}