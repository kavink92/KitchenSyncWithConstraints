//
// Created by kavin on 7/2/20.
//

#ifndef KITCHENSYNC_KITCHENORDERQUEUE_H
#define KITCHENSYNC_KITCHENORDERQUEUE_H


#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include "Order.h"

class KitchenOrderQueue {
public:
    KitchenOrderQueue() {}

    void AddOrder(std::unique_ptr<Order> order);

    std::unique_ptr<Order> GetNextOrders();

    void ShutDown();

private:
    int pending_order_count_ = 0;
    std::vector<std::unique_ptr<Order>> pending_orders_;
    std::mutex mtx_;
    std::condition_variable cv_;
    bool is_shut_down_ = false;
};


#endif //KITCHENSYNC_KITCHENORDERQUEUE_H
