//
// Created by kavin on 7/1/20.
//

#ifndef KITCHENSYNC_COURIERORDERQUEUE_H
#define KITCHENSYNC_COURIERORDERQUEUE_H


#include <vector>
#include <mutex>
#include <condition_variable>
#include "Order.h"

class CourierOrderQueue {
public:
    CourierOrderQueue() {}

    // Adds new order to the queue.
    void AddOrder(std::unique_ptr<Order> order);

    // Get the next available order from the queue.
    std::unique_ptr<Order> GetNextOrders();

private:
    int pending_order_count_ = 0;
    // Stores the pending orders in the queue.
    std::vector<std::unique_ptr<Order>> pending_orders_;
    std::mutex mtx_;
    std::condition_variable cv_;
};


#endif //KITCHENSYNC_COURIERORDERQUEUE_H
