//
// Created by kavin on 6/28/20.
//

#include <iostream>
#include <algorithm>
#include <thread>
#include "Shelves.h"
#include "flags.h"

bool SingleShelf::TryAdd(const std::string &order_id, std::unique_ptr<Food> food) {
    Lock();
    if (count_ == capacity_) {
        Unlock();
        return false;
    }
    UnsafeAdd(std::move(food));
    Unlock();
    return true;
}

bool SingleShelf::UnsafeAdd(std::unique_ptr<Food> food) {
    auto order_id = food->Id();
    if (content_.find(order_id) != content_.end()) {
        return false;
    } else {
        food->UpdateShelf(type_);
        content_.emplace(std::make_pair(order_id, std::move(food)));
        count_++;
        return true;
    }
}

std::unique_ptr<Food> SingleShelf::GetFood(const Order &order) {
    Lock();
    auto ret = UnsafeRemove(order.Id());
    Unlock();
    return std::move(ret);
}

std::unique_ptr<Food> SingleShelf::UnsafeRemove(const std::string &order_id) {
    auto it = content_.find(order_id);
    std::unique_ptr<Food> returned_food;
    returned_food.swap(it->second);
    content_.erase(it);
    return std::move(returned_food);
}


std::vector<std::unique_ptr<std::pair<Order, double>>> OverflowShelf::GetRemovalChoices() {
    // SHOUT: Modify this logic.
    std::vector<std::unique_ptr<std::pair<Order, double>>> choices;
    for (const auto &order_food : content_) {
        auto now = std::chrono::system_clock::now();
        std::chrono::system_clock::time_point min, max;
        if (now > order_food.second->CookedTime() + std::chrono::seconds(flags::kMinPickupTime)) {
            max = now;
        } else {
            max = order_food.second->CookedTime() + std::chrono::seconds(flags::kMinPickupTime);
        }

        if (flags::kMaxPickupTime < order_food.second->GetOrder().Shelflife()) {
            min = order_food.second->CookedTime() + std::chrono::seconds(flags::kMaxPickupTime);
        } else {
            min = order_food.second->CookedTime() + std::chrono::seconds(order_food.second->GetOrder().Shelflife());
        }

        const auto &food_copy = *order_food.second;
        double mean_value = (food_copy.ValueAtTime(min) - food_copy.ValueAtTime(max)) / 2;
        auto temp = order_food.second->ValueAtTime(
                std::chrono::system_clock::now() + std::chrono::seconds(1));
        auto choice = std::make_unique<std::pair<Order, double>>(
                std::make_pair(order_food.second->GetOrder(), mean_value));
        choices.emplace_back(std::move(choice));
    }

    std::sort(choices.begin(), choices.end(), [](const auto &a, const auto &b) -> bool {
        return a->second > b->second;
    });
    return choices;
}

Shelves::Shelves(std::shared_ptr<Logger> logger) : hot_shelf_(std::make_unique<SingleShelf>("hot", 10)),
                                                   cold_shelf_(std::make_unique<SingleShelf>("cold", 10)),
                                                   frozen_shelf_(std::make_unique<SingleShelf>("frozen", 10)),
                                                   overflow_shelf_(std::make_unique<OverflowShelf>("overflow", 15)),
                                                   logger_(logger) {}

void Shelves::AddFood(std::unique_ptr<Food> food) {
    if (food == nullptr) return;

    bool is_added;
    // TODO: Remove food copy
    auto food_cpy = *food;

    if (food->Temp() == "hot") {
        auto food_id = food->Id();
        is_added = hot_shelf_->TryAdd(food_id, std::move(food));
    } else if (food->Temp() == "cold") {
        auto food_id = food->Id();
        is_added = cold_shelf_->TryAdd(food_id, std::move(food));
    } else if (food->Temp() == "frozen") {
        auto food_id = food->Id();
        is_added = frozen_shelf_->TryAdd(food_id, std::move(food));
    }

    if (!is_added) {
        auto overflowed_food = std::make_unique<Food>(food_cpy);
        if (!overflow_shelf_->TryAdd(food_cpy.Id(), std::move(overflowed_food))) {
            overflow_shelf_->Lock();
            double best_value = 0;
            std::string best_order = "";
            auto choices = overflow_shelf_->GetRemovalChoices();

            bool was_replaced = false;
            for (const auto &choice: choices) {
                if (choice->first.Temp() == "hot") {
                    hot_shelf_->Lock();
                    if (!hot_shelf_->UnsafeIsFull()) {
                        auto removed_food = overflow_shelf_->UnsafeRemove(choice->first.Id());
                        order_shelf_map_.insert(std::make_pair(removed_food->Id(), "hot"));
                        hot_shelf_->UnsafeAdd(std::move(removed_food));
                        hot_shelf_->Unlock();

                        overflow_shelf_->UnsafeAdd(std::make_unique<Food>(food_cpy));
                        order_shelf_map_.insert(std::make_pair(food_cpy.Id(), "overflow"));
                        was_replaced = true;
                        break;
                    }
                    hot_shelf_->Unlock();
                } else if (choice->first.Temp() == "cold") {
                    cold_shelf_->Lock();
                    if (!cold_shelf_->UnsafeIsFull()) {
                        auto removed_food = overflow_shelf_->UnsafeRemove(choice->first.Id());
                        order_shelf_map_.insert(std::make_pair(removed_food->Id(), "cold"));
                        cold_shelf_->UnsafeAdd(std::move(removed_food));
                        cold_shelf_->Unlock();

                        overflow_shelf_->UnsafeAdd(std::make_unique<Food>(food_cpy));
                        order_shelf_map_.insert(std::make_pair(food_cpy.Id(), "overflow"));
                        was_replaced = true;
                        break;
                    }
                    cold_shelf_->Unlock();
                } else if (choice->first.Temp() == "frozen") {
                    frozen_shelf_->Lock();
                    if (!frozen_shelf_->UnsafeIsFull()) {
                        auto removed_food = overflow_shelf_->UnsafeRemove(choice->first.Id());
                        order_shelf_map_.insert(std::make_pair(removed_food->Id(), "frozen"));
                        frozen_shelf_->UnsafeAdd(std::move(removed_food));
                        frozen_shelf_->Unlock();

                        overflow_shelf_->UnsafeAdd(std::make_unique<Food>(food_cpy));
                        order_shelf_map_.insert(std::make_pair(food_cpy.Id(), "overflow"));
                        was_replaced = true;
                        break;
                    }
                    frozen_shelf_->Unlock();
                }
            }

            if (!was_replaced) {
                auto removed_food = overflow_shelf_->UnsafeRemove(choices[0]->first.Id());
                auto it = order_shelf_map_.find(choices[0]->first.Id());
                it->second = "discard";

                overflow_shelf_->UnsafeAdd(std::make_unique<Food>(food_cpy));
                order_shelf_map_.insert(std::make_pair(food_cpy.Id(), "overflow"));
            }


            overflow_shelf_->Unlock();

        } else {
            order_shelf_map_.insert(std::make_pair(food_cpy.Id(), "overflow"));
        }

    } else {
        order_shelf_map_.insert(std::make_pair(food_cpy.Id(), food_cpy.Temp()));
    }
}

std::unique_ptr<Food> Shelves::GetFood(const Order &order) {
    std::this_thread::sleep_for(std::chrono::seconds(4));

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
        // Remove this.
        food->PickedUp();
        return std::move(food);
    } else if (ret_iter->second == "cold") {
        auto food = cold_shelf_->GetFood(order);
        auto time2 = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        logger_->Log("Food with Id: " + order.Id() + " was picked up at " + std::ctime(&time2) + " with value " +
                     std::to_string(food->ValueNow()));
        food->PickedUp();
        return std::move(food);
    } else if (ret_iter->second == "frozen") {
        auto food = frozen_shelf_->GetFood(order);
        auto time2 = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        logger_->Log("Food with Id: " + order.Id() + " was picked up at " + std::ctime(&time2) + " with value " +
                     std::to_string(food->ValueNow()));
        food->PickedUp();
        return std::move(food);
    } else if (ret_iter->second == "overflow") {
        auto food = overflow_shelf_->GetFood(order);
        auto time2 = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        logger_->Log("Food with Id: " + order.Id() + " was picked up at " + std::ctime(&time2) + " with value " +
                     std::to_string(food->ValueNow()));
        food->PickedUp();
        return std::move(food);
    } else if (ret_iter->second == "discard") {
        logger_->Log("Food with Id: " + order.Id() + " was discarded.");
        return nullptr;
    }
}

std::string FindFoodToRemove() {

    return "";
}
