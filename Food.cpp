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

std::chrono::system_clock::time_point Food::CookedTime() {
    return cooked_time_;
}

void Food::UpdateShelf(const std::string &type) {
    shelf_history_.push_back(std::make_pair(type, std::chrono::system_clock::now()));

    auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto print_str =
            "Food with Id: " + order_.Id() + " placed in shelf: " + type + " at time " + std::ctime(&time) + "\n";
    //std::cout << print_str;
}

double Food::ValueNow() const {
    return ValueAtTime(std::chrono::system_clock::now());
}

double Food::ValueAtTime(const std::chrono::system_clock::time_point &time) const {
    std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - cooked_time_;
    if (elapsed_seconds.count() > order_.Shelflife()) {
        return 0;
    }

    if (shelf_history_.empty()) {
        return 1;
    }

    auto calc_type = shelf_history_[0].first;
    auto calc_time = shelf_history_[0].second;
    double value = 1;
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

    std::chrono::duration<double> duration = time - calc_time;
    value -= Calc(order_.Shelflife(), order_.DecayRate(), duration.count(),
                  ShelfDecayModifier(calc_type));
    if (value <= 0) {
        return 0;
    } else {
        return value;
    }
}

void Food::PickedUp() {
    is_picked_up = true;
    pickup_time_ = std::chrono::system_clock::now();
    value_at_pickup_ = ValueNow();
}

bool Food::WasPickedUp() const {
    return is_picked_up;
}

double Food::ValueAtPickup() const {
    return value_at_pickup_;
}

void Food::Delivered() {
    is_delivered = true;
    delivery_time_ = std::chrono::system_clock::now();
}

double Food::Calc(const int shelf_life, const double decay_rate, const double order_age,
                  const double shelf_decay_modifier) const {
    return static_cast<double>(decay_rate * order_age * shelf_decay_modifier) / shelf_life;
}
