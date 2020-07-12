//
// Created by kavin on 6/28/20.
//

#include "Food.h"

std::string Food::Id() const {
    return order_.Id();
}

Order Food::GetOrder() const {
    return order_;
}

std::string Food::Temp() const {
    return order_.Temp();
}

void Food::UpdateShelf(const std::string &type) {
    shelf_history_.push_back(std::make_pair(type, std::chrono::system_clock::now()));

    auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto print_str =
            "Food with Id: " + order_.Id() + " placed in shelf: " + type + " at time " + std::ctime(&time) + "\n";
    std::cout << print_str;
}

double Food::ValueNow() const {
    return ValueAtTime(std::chrono::system_clock::now());
}

double Food::ValueAtTime(const std::chrono::system_clock::time_point &time) const {
    std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - cooked_time_;
    // Food has expired.
    if (elapsed_seconds.count() > order_.Shelflife()) {
        return 0;
    }

    // Food is in transition state and not moved to any shelf.
    if (shelf_history_.empty()) {
        return 1;
    }

    auto calc_type = shelf_history_[0].first;
    auto calc_time = shelf_history_[0].second;
    double value = 1;

    // At the end of each iteration of the loop, value would contain the value of the food before it was shifted from
    // i-1th shelf to ith shelf.
    for (int i = 1; i < shelf_history_.size(); i++) {
        std::chrono::duration<double> duration = shelf_history_[i].second - calc_time;
        value -= Calc(order_.Shelflife(), order_.DecayRate(), duration.count(),
                      ShelfDecayModifier(calc_type));
        auto calc_type = shelf_history_[i].first;
        auto calc_time = shelf_history_[i].second;

        if (value <= 0) {
            return 0;
        }
    }

    // The food is still in the last shelf of the shelf history and calculate the value at given time.
    std::chrono::duration<double> duration = time - calc_time;
    value -= Calc(order_.Shelflife(), order_.DecayRate(), duration.count(),
                  ShelfDecayModifier(calc_type));
    if (value <= 0) {
        return 0;
    } else {
        return value;
    }
}

double Food::Calc(const int shelf_life, const double decay_rate, const double order_age,
                  const double shelf_decay_modifier) const {
    return static_cast<double>(decay_rate * order_age * shelf_decay_modifier) / shelf_life;
}
