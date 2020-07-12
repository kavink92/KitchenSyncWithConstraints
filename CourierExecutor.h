//
// Created by kavin on 6/28/20.
//

#ifndef KITCHENSYNC_COURIEREXECUTOR_H
#define KITCHENSYNC_COURIEREXECUTOR_H

#include <chrono>
#include <future>
#include <vector>
#include <unordered_map>

using namespace std::chrono;

#include "Shelves.h"
#include "CourierOrderQueue.h"
#include "Logger.h"

class CourierExecutor {
public:
    CourierExecutor(std::shared_ptr<Shelves> shelves, std::shared_ptr<CourierOrderQueue> cod,
                    std::shared_ptr<Logger> logger);

    void Run();

    void UpdateTotalValue(double food_value) {
        mtx_->lock();
        *total_value_ += food_value;
        mtx_->unlock();
    }

    double TotalValue() {
        return *total_value_;
    };

private:
    std::unordered_map<std::string, std::pair<Order, bool>> order_and_status_;
    std::shared_ptr<Shelves> shelves_;
    std::shared_ptr<CourierOrderQueue> cod_;
    std::shared_ptr<Logger> logger_;

    std::shared_ptr<std::mutex> mtx_;
    std::shared_ptr<double> total_value_ = 0;
};


#endif //KITCHENSYNC_COURIEREXECUTOR_H
