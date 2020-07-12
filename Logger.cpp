//
// Created by kavin on 7/4/20.
//

#include <iostream>
#include "Logger.h"

void Logger::Log(const std::string &message) {
    // Ensures the cout threads are synchronized. This would not allow the output messages to be intermingled.
    std::unique_lock<std::mutex> lck(mtx_);
    std::cout << message << std::endl;
}
