//
// Created by kavin on 6/27/20.
//

#ifndef KITCHENSYNC_ORDERGENERATOR_H
#define KITCHENSYNC_ORDERGENERATOR_H


#include "Kitchen.h"
#include "nlohmann/json.hpp"
#include "CourierExecutor.h"
#include "CourierOrderQueue.h"

class OrderGenerator {
public:
    OrderGenerator(std::shared_ptr<KitchenOrderQueue> kitchen_order_queue,
                   std::shared_ptr<CourierOrderQueue> courier_order_queue, std::shared_ptr<Logger> logger);

    // Generates the orders.
    void Generate();

private:
    std::shared_ptr<KitchenOrderQueue> kitchen_order_queue_;
    std::shared_ptr<CourierOrderQueue> courier_order_queue_;
    std::shared_ptr<Logger> logger_;
};


#endif //KITCHENSYNC_ORDERGENERATOR_H
