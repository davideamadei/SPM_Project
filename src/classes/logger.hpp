#pragma once

#include <chrono>
#include <string>
#include <map>

class Timer{
    private:
        std::chrono::system_clock::time_point start_time;
        std::chrono::system_clock::time_point stop_time;
        std::string timer_str;
    public:
        void start(std::string msg);
        long stop();
        std::string getStr();

};

class Logger{
    private:
        Timer timer;
        std::map<std::string, double> cumulative_stats;
    public:
        void compute_avg_stats(int n_tries);
        void write_logs(std::string filename, int n_tries);
        void start(std::string stat_name);
        long stop();
};