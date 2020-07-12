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
    OrderGenerator(std::shared_ptr<Shelves> shelves, std::shared_ptr<KitchenOrderQueue> kod,
                   std::shared_ptr<CourierOrderQueue> cod, std::shared_ptr<Logger> logger);
    // Generates the orders.
    void Generate();

private:
    std::shared_ptr<Shelves> shelves_;
    std::shared_ptr<KitchenOrderQueue> kod_;
    std::shared_ptr<CourierOrderQueue> cod_;
    std::shared_ptr<Logger> logger_;
};


#endif //KITCHENSYNC_ORDERGENERATOR_H
