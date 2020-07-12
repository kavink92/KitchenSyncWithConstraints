//
// Created by kavin on 6/27/20.
//

#include <iostream>
#include "Kitchen.h"


Kitchen::Kitchen(std::shared_ptr<Shelves> shelves, std::shared_ptr<KitchenOrderQueue> kod,
                 std::shared_ptr<Logger> logger) : shelves_(shelves), kod_(kod), logger_(logger) {}

void Kitchen::Run() {

    while (true) {
        auto next_order = kod_->GetNextOrders();
        if (!next_order) {
            return;
        }

        auto food = std::make_unique<Food>(*next_order);

        shelves_->AddFood(std::move(food));
    }
}