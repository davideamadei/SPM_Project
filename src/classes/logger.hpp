/**
 * @file logger.hpp
 * @author Davide Amadei (davide.amadei97@gmail.com)
 * @brief header file containing the logging classes
 * @date 2023-10-04
 * 
 */
#pragma once

#include <chrono>
#include <string>
#include <map>

/**
 * @brief class implementing a timer for logging purposes
 * 
 */
class Timer{
    private:
        /**
         * @brief starting time
         * 
         */
        std::chrono::system_clock::time_point start_time;
        /**
         * @brief ending time
         * 
         */
        std::chrono::system_clock::time_point stop_time;
        /**
         * @brief string representing the timer
         * 
         */
        std::string timer_str;
    public:
        void start(std::string msg);
        long stop();
        std::string getStr();

};

/**
 * @brief class implementing various logging utilities
 * 
 */
class Logger{
    private:
        /**
         * @brief timer obect to track time
         * 
         */
        Timer timer;
        /**
         * @brief map containing the cumulative stats gathered so far
         * 
         */
        std::map<std::string, double> cumulative_stats;
    public:
        void write_logs(std::string filename, int n_tries, int n_threads);
        void start(std::string stat_name);
        void add_stat(std::string stat_name, long time);
        long stop();

};