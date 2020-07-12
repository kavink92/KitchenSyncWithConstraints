//
// Created by kavin on 7/2/20.
//

#include <iostream>
#include "KitchenOrderQueue.h"

void KitchenOrderQueue::AddOrder(std::unique_ptr<Order> order) {
    std::string order_id = order->Id();
    pending_orders_.emplace_back(std::move(order));
    pending_order_count_++;
    while (pending_order_count_ == 1) {
        cv_.notify_one();
    }

}

std::unique_ptr<Order> KitchenOrderQueue::GetNextOrders() {
    std::unique_lock<std::mutex> lck(mtx_);
    while (pending_order_count_ == 0 && !is_shut_down_) {
        cv_.wait(lck);
        if (is_shut_down_ && pending_order_count_ == 0) {
            return nullptr;
        }
    }

    if (is_shut_down_ && pending_order_count_ == 0) {
        return nullptr;
    }

    std::unique_ptr<Order> order;
    order.swap(pending_orders_[0]);
    pending_orders_.erase(pending_orders_.begin());
    pending_order_count_--;

    return order;
}

void KitchenOrderQueue::ShutDown() {
    is_shut_down_ = true;
    cv_.notify_all();
}