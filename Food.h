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
    // Returns the decay modifier based on which shelf the food is stored in.
    double ShelfDecayModifier(const std::string &type) {
        if (type == "overflow") {
            return 2;
        } else {
            return 1;
        }
    }
}  // namespace

class Food {
public:
    Food(const Order &order) : order_(order) {}

    // Returns the Order Id of the food.
    std::string Id() const;

    // Returns the Order Temp of the food.
    std::string Temp() const;

    // Gets the order corresponding the prepared food.
    Order GetOrder() const;

    // Update the shelf to which the food is currently kept.
    void UpdateShelf(const std::string &type);

    // Get the value of the food currently.
    double ValueNow() const;

    // Get the value of this good at given time.
    double ValueAtTime(const std::chrono::system_clock::time_point &time) const;

    std::chrono::system_clock::time_point CookedTime() const {
        return cooked_time_;
    }

private:
    // Calculates the decrease in value of the food based based on the given parameters.
    double Calc(const int shelf_life, const double decay_rate, const double order_age,
                const double shelf_decay_modifier) const;

    // Order corresponding to this food.
    Order order_;
    // Time when the food was prepared.
    const std::chrono::system_clock::time_point cooked_time_ = std::chrono::system_clock::now();
    // History of when the food was moved to a given shelf. Example {"cold", time_point_1}
    std::vector<std::pair<std::string, const std::chrono::system_clock::time_point>> shelf_history_;
};


#endif //KITCHENSYNC_FOOD_H
