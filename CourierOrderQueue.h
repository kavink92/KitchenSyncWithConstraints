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
    // constructor delete

    CourierOrderQueue() {}

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


#endif //KITCHENSYNC_COURIERORDERQUEUE_H
