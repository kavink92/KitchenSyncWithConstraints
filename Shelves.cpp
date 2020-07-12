//
// Created by kavin on 6/28/20.
//

#include <iostream>
#include <algorithm>
#include <thread>
#include "Shelves.h"
#include "flags.h"

bool Shelves::SingleRowShelf::TryAdd(std::unique_ptr<Food> food) {
    // Acquires lock for the shelf.
    Lock();
    if (content_.size() == capacity_) {
        Unlock();
        // Shelf is full and hence the food cannot be added.
        return false;
    }
    bool was_added = UnsafeAdd(std::move(food));
    Unlock();
    return was_added;
}

bool Shelves::SingleRowShelf::UnsafeAdd(std::unique_ptr<Food> food) {
    auto order_id = food->Id();
    if (content_.find(order_id) != content_.end()) {
        // The food was already added to the shelf.
        return false;
    } else {
        // The food is being added to the shelf.
        food->UpdateShelf(type_);
        content_.emplace(std::make_pair(order_id, std::move(food)));
        return true;
    }
}

std::unique_ptr<Food> Shelves::SingleRowShelf::GetFood(const Order &order) {
    Lock();
    auto food = UnsafeRemove(order.Id());
    Unlock();
    return std::move(food);
}

std::unique_ptr<Food> Shelves::SingleRowShelf::UnsafeRemove(const std::string &order_id) {
    auto it = content_.find(order_id);
    if (it == content_.end()) {
        return nullptr;
    }

    std::unique_ptr<Food> removed_food;
    removed_food.swap(it->second);
    content_.erase(it);
    return std::move(removed_food);
}


std::vector<std::unique_ptr<std::pair<Order, double>>> Shelves::OverflowShelf::GetRemovalChoices() {
    std::vector<std::unique_ptr<std::pair<Order, double>>> removal_choices;
    // For each food item present in the shelf, probabilistically estimate the value of the food its time of delivery.
    // We do not know the exact time at which the food would be picked up but we know what is the time frame when
    // the courier would arrive.
    for (const auto &order_food : content_) {
        auto now = std::chrono::system_clock::now();
        // Time bounds within which the food item would have to be picke up.
        std::chrono::system_clock::time_point earliest_time_for_pickup, farthest_time_for_pickup;

        double mean_value_at_delivery;
        if (order_food.second->GetOrder().Shelflife() < flags::kMaxPickupTime) {
            mean_value_at_delivery = 0;
        } else {
            if (now > order_food.second->CookedTime() + std::chrono::seconds(flags::kMinPickupTime)) {
                // The food could be picked up any time now.
                earliest_time_for_pickup = now;
            } else {
                // The food would not be picked up now but will be picked up after earliest_time_for_pickup.
                earliest_time_for_pickup =
                        order_food.second->CookedTime() + std::chrono::seconds(flags::kMinPickupTime);
            }

            const Food *removal_candidate = order_food.second.get();
            farthest_time_for_pickup = order_food.second->CookedTime() + std::chrono::seconds(flags::kMaxPickupTime);
            // The food would be picked up before the food expires.
            if (flags::kMaxPickupTime <= order_food.second->GetOrder().Shelflife()) {
                mean_value_at_delivery = (removal_candidate->ValueAtTime(earliest_time_for_pickup) -
                                          removal_candidate->ValueAtTime(farthest_time_for_pickup)) / 2;
            } else {
                // There is a possibility that the food could expire before it is picked up.
                std::chrono::system_clock::time_point expiry_time = order_food.second->CookedTime() +
                                                                    std::chrono::seconds(
                                                                            order_food.second->GetOrder().Shelflife());
                farthest_time_for_pickup =
                        order_food.second->CookedTime() + std::chrono::seconds(flags::kMaxPickupTime);
                std::chrono::duration<double> duration_earliest_pickup_to_expiry =
                        expiry_time - earliest_time_for_pickup;
                std::chrono::duration<double> duration_earliest_pickup_to_fartheset_pickup =
                        farthest_time_for_pickup - earliest_time_for_pickup;
                // Weighted average of value of the food before expiry and after expirty. Note the value of the food
                // becomes zero after expiry.
                mean_value_at_delivery = ((removal_candidate->ValueAtTime(earliest_time_for_pickup) -
                                           removal_candidate->ValueAtTime(farthest_time_for_pickup)) *
                                          duration_earliest_pickup_to_expiry / 2) /
                                         (duration_earliest_pickup_to_fartheset_pickup);
            }
        }

        auto removal_choice = std::make_unique<std::pair<Order, double>>(
                std::make_pair(order_food.second->GetOrder(), mean_value_at_delivery));
        removal_choices.emplace_back(std::move(removal_choice));
    }

    std::sort(removal_choices.begin(), removal_choices.end(), [](const auto &a, const auto &b) -> bool {
        return a->second > b->second;
    });
    return removal_choices;
}

Shelves::Shelves(std::shared_ptr<Logger> logger) : hot_shelf_(std::make_unique<SingleRowShelf>("hot", 10)),
                                                   cold_shelf_(std::make_unique<SingleRowShelf>("cold", 10)),
                                                   frozen_shelf_(std::make_unique<SingleRowShelf>("frozen", 10)),
                                                   overflow_shelf_(std::make_unique<OverflowShelf>("overflow", 15)),
                                                   logger_(logger) {}

void Shelves::AddFood(std::unique_ptr<Food> food) {
    const auto &order = food->GetOrder();
    const auto &food_to_add = *food;
    if (food == nullptr) return;

    bool was_added_to_correct_temperature_shelf = true;
    if (food->Temp() == "hot") {
        auto food_id = food->Id();
        was_added_to_correct_temperature_shelf = hot_shelf_->TryAdd(std::make_unique<Food>(food_to_add));
    } else if (food->Temp() == "cold") {
        auto food_id = food->Id();
        was_added_to_correct_temperature_shelf = cold_shelf_->TryAdd(std::make_unique<Food>(food_to_add));
    } else if (food->Temp() == "frozen") {
        auto food_id = food->Id();
        was_added_to_correct_temperature_shelf = frozen_shelf_->TryAdd(std::make_unique<Food>(food_to_add));
    }

    if (!was_added_to_correct_temperature_shelf) {
        // The food could not be added to the shelf corresponding to it's temperature. Hence trying to add to overflow
        // shelf.
        if (!overflow_shelf_->TryAdd(std::make_unique<Food>(food_to_add))) {
            // The overflow shelf is full as well and hence we need to shift or discard one of the food items from
            // the overflow shelf and add the food_to_add to it.

            // Lock the already full overflow shelf so that no new attempts to add to the shelf are fulfilled till we
            // // add the current food.
            overflow_shelf_->Lock();
            auto removal_choices = overflow_shelf_->GetRemovalChoices();

            bool should_discard_from_overflow = true;
            for (const auto &removal_choice: removal_choices) {
                if (removal_choice->first.Temp() == "hot") {
                    hot_shelf_->Lock();
                    if (!hot_shelf_->UnsafeIsFull()) {
                        //
                        auto removed_food = overflow_shelf_->UnsafeRemove(removal_choice->first.Id());
                        order_shelf_map_.insert(std::make_pair(removed_food->Id(), "hot"));
                        hot_shelf_->UnsafeAdd(std::move(removed_food));
                        hot_shelf_->Unlock();

                        if (overflow_shelf_->UnsafeAdd(std::make_unique<Food>(food_to_add))) {
                            order_shelf_map_.insert(std::make_pair(order.Id(), "overflow"));
                            should_discard_from_overflow = false;
                            break;
                        }
                    }
                    hot_shelf_->Unlock();
                } else if (removal_choice->first.Temp() == "cold") {
                    cold_shelf_->Lock();
                    if (!cold_shelf_->UnsafeIsFull()) {
                        auto removed_food = overflow_shelf_->UnsafeRemove(removal_choice->first.Id());
                        order_shelf_map_.insert(std::make_pair(removed_food->Id(), "cold"));
                        cold_shelf_->UnsafeAdd(std::move(removed_food));
                        cold_shelf_->Unlock();

                        if (overflow_shelf_->UnsafeAdd(std::make_unique<Food>(food_to_add))) {
                            order_shelf_map_.insert(std::make_pair(order.Id(), "overflow"));
                            should_discard_from_overflow = false;
                            break;
                        }
                    }
                    cold_shelf_->Unlock();
                } else if (removal_choice->first.Temp() == "frozen") {
                    frozen_shelf_->Lock();
                    if (!frozen_shelf_->UnsafeIsFull()) {
                        auto removed_food = overflow_shelf_->UnsafeRemove(removal_choice->first.Id());
                        order_shelf_map_.insert(std::make_pair(removed_food->Id(), "frozen"));
                        frozen_shelf_->UnsafeAdd(std::move(removed_food));
                        frozen_shelf_->Unlock();

                        if (overflow_shelf_->UnsafeAdd(std::make_unique<Food>(food_to_add))) {
                            order_shelf_map_.insert(std::make_pair(order.Id(), "overflow"));
                            should_discard_from_overflow = false;
                            break;
                        }
                    }
                    frozen_shelf_->Unlock();
                }
            }

            if (should_discard_from_overflow) {
                auto removed_food = overflow_shelf_->UnsafeRemove(removal_choices[0]->first.Id());
                auto it = order_shelf_map_.find(removal_choices[0]->first.Id());
                it->second = "discard";

                overflow_shelf_->UnsafeAdd(std::make_unique<Food>(food_to_add));
                order_shelf_map_.insert(std::make_pair(order.Id(), "overflow"));
            }


            overflow_shelf_->Unlock();

        } else {
            // Update the map with which shelf the food was added to.
            order_shelf_map_.insert(std::make_pair(order.Id(), "overflow"));
        }

    } else {
        // Update the map with which shelf the food was added to.
        order_shelf_map_.insert(std::make_pair(order.Id(), order.Temp()));
    }
}

std::unique_ptr<Food> Shelves::GetFood(const Order &order) {
    auto ret_iter = order_shelf_map_.find(order.Id());
    if (ret_iter == order_shelf_map_.end()) {
        logger_->Log("Did not get food for Id: " + order.Id());
        return nullptr;
    }

    if (ret_iter->second == "hot") {
        auto food = hot_shelf_->GetFood(order);
        auto time2 = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        logger_->Log("Food with Id: " + order.Id() + " was picked up at " + std::ctime(&time2) + " with value " +
                     std::to_string(food->ValueNow()));
        return std::move(food);
    } else if (ret_iter->second == "cold") {
        auto food = cold_shelf_->GetFood(order);
        auto time2 = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        logger_->Log("Food with Id: " + order.Id() + " was picked up at " + std::ctime(&time2) + " with value " +
                     std::to_string(food->ValueNow()));
        return std::move(food);
    } else if (ret_iter->second == "frozen") {
        auto food = frozen_shelf_->GetFood(order);
        auto time2 = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        logger_->Log("Food with Id: " + order.Id() + " was picked up at " + std::ctime(&time2) + " with value " +
                     std::to_string(food->ValueNow()));
        return std::move(food);
    } else if (ret_iter->second == "overflow") {
        auto food = overflow_shelf_->GetFood(order);
        auto time2 = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        logger_->Log("Food with Id: " + order.Id() + " was picked up at " + std::ctime(&time2) + " with value " +
                     std::to_string(food->ValueNow()));
        return std::move(food);
    } else if (ret_iter->second == "discard") {
        logger_->Log("Food with Id: " + order.Id() + " was discarded.");
        return nullptr;
    }
}
