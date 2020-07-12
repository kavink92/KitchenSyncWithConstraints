//
// Created by kavin on 6/28/20.
//

#ifndef KITCHENSYNC_SHELVES_H
#define KITCHENSYNC_SHELVES_H


#include <set>
#include <memory>
#include <condition_variable>
#include <unordered_map>
#include "Food.h"
#include "Logger.h"

class SingleShelf {
public:
    SingleShelf(const std::string &type, const int capacity) : type_(type), capacity_(capacity) {}

    std::unique_ptr<Food> GetFood(const Order &order);

    void Lock() {
        mtx_.lock();
    }

    void Unlock() {
        mtx_.unlock();
    }

    bool TryAdd(const std::string &order_id, std::unique_ptr<Food> food);

    // Do not use them without perfoming lock on the contents of the shelf.
    bool UnsafeIsFull() {
        return count_ == capacity_;
    }

    std::unique_ptr<Food> UnsafeRemove(const std::string &order_id);

    bool UnsafeAdd(std::unique_ptr<Food> food);

protected:
    std::unordered_map<std::string, std::unique_ptr<Food>> content_;

private:
    int count_ = 0;
    std::string type_;
    const int capacity_;
    std::mutex mtx_;
    std::condition_variable cv_;
};

class OverflowShelf : public SingleShelf {
public:
    OverflowShelf(const std::string &type, const int capacity) : SingleShelf(type, capacity) {}

    std::unique_ptr<Food> RemoveAndAdd(const std::string &removed_food_order_id, std::unique_ptr<Food> food);

    std::vector<std::unique_ptr<std::pair<Order, double>>> GetRemovalChoices();

};

class Shelves {
public:
    Shelves(std::shared_ptr<Logger> logger);

    void AddFood(std::unique_ptr<Food> food);

    std::unique_ptr<Food> GetFood(const Order &order);

private:
    std::string FindFoodToRemove();

    std::unique_ptr<SingleShelf> hot_shelf_;
    std::unique_ptr<SingleShelf> cold_shelf_;
    std::unique_ptr<SingleShelf> frozen_shelf_;
    std::unique_ptr<OverflowShelf> overflow_shelf_;

    std::unordered_map<std::string, std::string> order_shelf_map_;
    std::shared_ptr<Logger> logger_;

};


#endif //KITCHENSYNC_SHELVES_H
