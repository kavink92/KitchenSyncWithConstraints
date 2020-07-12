//
// Created by kavin on 6/28/20.
//

#include <iostream>
#include "CourierExecutor.h"

CourierExecutor::CourierExecutor(std::shared_ptr<Shelves> shelves,
                                 std::shared_ptr<CourierOrderQueue> courier_order_queue,
                                 std::shared_ptr<Logger> logger) : shelves_(shelves),
                                                                   courier_order_queue_(courier_order_queue),
                                                                   logger_(logger) {}

void CourierExecutor::Run() {

    while (true) {
        // Get the next available order. If not order is currently available, it waits for an order to be added to the
        // queue.
        auto next_order = courier_order_queue_->GetNextOrders();

        // TODO: Randomize it based on the consts in flags.h
        std::this_thread::sleep_for(std::chrono::seconds(4));

        auto food = shelves_->GetFood(*next_order);
        if (food == nullptr) {
            logger_->Log("Could not get order: " + next_order->Id());
        } else {
            auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            logger_->Log(
                    "Food with Id: " + next_order->Id() + " was picked up at " + std::ctime(&time) + " with value " +
                    std::to_string(food->ValueNow()));

            // Instantly Deliver Food.
        }
    }
}
