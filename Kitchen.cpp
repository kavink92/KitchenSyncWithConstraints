//
// Created by kavin on 6/27/20.
//

#include <iostream>
#include "Kitchen.h"


Kitchen::Kitchen(std::shared_ptr<Shelves> shelves, std::shared_ptr<KitchenOrderQueue> kitchen_order_queue,
                 std::shared_ptr<Logger> logger) : shelves_(shelves), kitchen_order_queue_(kitchen_order_queue),
                                                   logger_(logger) {}

void Kitchen::Run() {

    while (true) {
        // Get the next available order. If not order is currently available, it waits for an order to be added to the
        // queue.
        auto next_order = kitchen_order_queue_->GetNextOrders();

        // Cook the food.
        auto food = std::make_unique<Food>(*next_order);

        // Adding the food to the shelf.
        shelves_->AddFood(std::move(food));
    }
}