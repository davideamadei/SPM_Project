#include "logger.hpp"
#include <fstream>

void Timer::start(std::string msg){
    start_time = std::chrono::system_clock::now();
    timer_str = msg;
    return;
}

long Timer::stop(){
    stop_time = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = stop_time - start_time;
    auto elapsed_usec = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    return elapsed_usec;
}

std::string Timer::getStr(){return timer_str;}

void Logger::start(std::string stat_name){
    timer.start(stat_name);
    return;
}

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