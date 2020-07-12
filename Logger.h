//
// Created by kavin on 7/4/20.
//

#ifndef KITCHENSYNC_LOGGER_H
#define KITCHENSYNC_LOGGER_H


#include <mutex>
#include <memory>
#include <condition_variable>
#include <vector>


class Logger {
public:
    // Logs the given message.
    void Log(const std::string &message);

private:
    std::mutex mtx_;
};

#endif //KITCHENSYNC_LOGGER_H
