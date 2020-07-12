//
// Created by kavin on 7/1/20.
//

#include <iostream>
#include "CourierOrderQueue.h"

void CourierOrderQueue::AddOrder(std::unique_ptr<Order> order) {
    std::string order_id = order->Id();
    pending_orders_.emplace_back(std::move(order));
    pending_order_count_++;
    if (pending_order_count_ == 1) {
        // Notifies so that a thread which is waiting on GetNextOrders is now unblocked.
        cv_.notify_one();
    }
}

std::unique_ptr<Order> CourierOrderQueue::GetNextOrders() {
    std::unique_lock<std::mutex> lck(mtx_);
    while (pending_order_count_ == 0) {
        // Waits for at least one order to be present in pending_orders.
        cv_.wait(lck);
    }

    if (pending_order_count_ == 0) {
        return nullptr;
    }

    std::unique_ptr<Order> order;
    order.swap(pending_orders_[0]);
    pending_orders_.erase(pending_orders_.begin());
    pending_order_count_--;

    return std::move(order);
}