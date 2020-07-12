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

    // Add order to the queue.
    void AddOrder(std::unique_ptr<Order> order);

    // Get the next order from the queue.
    std::unique_ptr<Order> GetNextOrders();

private:
    int pending_order_count_ = 0;
    // Stores the pending orders in the queue.
    std::vector<std::unique_ptr<Order>> pending_orders_;
    std::mutex mtx_;
    std::condition_variable cv_;
};


#endif //KITCHENSYNC_KITCHENORDERQUEUE_H
