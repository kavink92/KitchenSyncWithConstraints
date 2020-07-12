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
    CourierExecutor(std::shared_ptr<Shelves> shelves, std::shared_ptr<CourierOrderQueue> courier_order_queue,
                    std::shared_ptr<Logger> logger);

    // Runs the core logic of Couriers. It receives next available order from courier_order_queue_ and dispatches a
    // courier to pick the food with a randomized delay. Thread safe.
    void Run();

private:
    // The following member variables are not owned by CourierExecutor.
    // Shelves from which the food will be picked.
    std::shared_ptr<Shelves> shelves_;
    // The order queue from which the next available order is obtained.
    std::shared_ptr<CourierOrderQueue> courier_order_queue_;
    std::shared_ptr<Logger> logger_;
};


#endif //KITCHENSYNC_COURIEREXECUTOR_H
