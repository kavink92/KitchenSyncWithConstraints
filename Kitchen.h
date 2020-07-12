//
// Created by kavin on 6/27/20.
//

#ifndef KITCHENSYNC_KITCHEN_H
#define KITCHENSYNC_KITCHEN_H

#include <vector>
#include <mutex>
#include <condition_variable>
#include <future>

#include "Order.h"
#include "Shelves.h"
#include "KitchenOrderQueue.h"
#include "Logger.h"

class Kitchen {
public:
    Kitchen(std::shared_ptr<Shelves> shelves, std::shared_ptr<KitchenOrderQueue> kitchen_order_queue,
            std::shared_ptr<Logger> logger);

    // Runs the kitchen and contains the core logic. It receives next available order from kitchen_order_queue_ and adds
    // cooked food to the shelf. Thread safe.
    void Run();

private:
    // The following member variables are not owned by Kitchen.
    // Shelves to which the food would be added.
    std::shared_ptr<Shelves> shelves_;
    // The order queue from which the next available order is obtained.
    std::shared_ptr<KitchenOrderQueue> kitchen_order_queue_;
    std::shared_ptr<Logger> logger_;
};


#endif //KITCHENSYNC_KITCHEN_H
