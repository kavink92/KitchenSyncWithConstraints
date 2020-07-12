//
// Created by kavin on 6/28/20.
//

#ifndef KITCHENSYNC_FOOD_H
#define KITCHENSYNC_FOOD_H


#include <chrono>
#include <vector>
#include <iostream>
#include "Order.h"

namespace {
    double ShelfDecayModifier(const std::string &type) {
        if (type == "overflow") {
            return 2;
        } else {
            return 1;
        }
    }
}

class Food {
public:
    Food(const Order &order) : order_(order) {}

    std::string Id() const;

    Order GetOrder() const;

    std::string Temp() const;

    std::chrono::system_clock::time_point CookedTime();

    void UpdateShelf(const std::string &type);

    double ValueNow() const;

    double ValueAtTime(const std::chrono::system_clock::time_point &time) const;

    void PickedUp();

    bool WasPickedUp() const;

    double ValueAtPickup() const;

    void Delivered();


private:
    double Calc(const int shelf_life, const double decay_rate, const double order_age,
                const double shelf_decay_modifier) const;

    Order order_;
    const std::chrono::system_clock::time_point cooked_time_ = std::chrono::system_clock::now();
    std::vector<std::pair<std::string, const std::chrono::system_clock::time_point>> shelf_history_;
    bool is_picked_up = false;
    double value_at_pickup_;
    std::chrono::system_clock::time_point pickup_time_;
    bool is_delivered = false;
    std::chrono::system_clock::time_point delivery_time_;

};


#endif //KITCHENSYNC_FOOD_H
