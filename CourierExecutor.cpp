//
// Created by kavin on 6/28/20.
//

#include <iostream>
#include "CourierExecutor.h"

CourierExecutor::CourierExecutor(std::shared_ptr<Shelves> shelves, std::shared_ptr<CourierOrderQueue> cod,
                                 std::shared_ptr<Logger> logger) : shelves_(shelves), cod_(cod), logger_(logger) {}

void CourierExecutor::Run() {

    while (true) {
        auto next_order = cod_->GetNextOrders();
        if (!next_order) {
            return;
        }


        std::this_thread::sleep_for(std::chrono::seconds(4));
        auto food = shelves_->GetFood(*next_order);
        if (food == nullptr) {
            logger_->Log("Could not get order: " + next_order->Id());
        } else {
            food->PickedUp();
            auto time2 = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            logger_->Log(
                    "Food with Id: " + next_order->Id() + " was picked up at " + std::ctime(&time2) + " with value " +
                    std::to_string(food->ValueNow()));

            // TODO: Deliver Food.
        }
    }
}
