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
    Kitchen(std::shared_ptr<Shelves> shelves, std::shared_ptr<KitchenOrderQueue> kod, std::shared_ptr<Logger> logger);

    void Run();

private:
    std::unordered_map<std::string, std::pair<Order, bool>> order_and_status_;
    std::shared_ptr<Shelves> shelves_;
    std::shared_ptr<KitchenOrderQueue> kod_;
    std::shared_ptr<Logger> logger_;
};


#endif //KITCHENSYNC_KITCHEN_H
