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

class Shelves {
public:
    Shelves(std::shared_ptr<Logger> logger);

    // Add food to the best possible shelf. The logic is summarized here.
    // 1) Add the food to its specified temperature shelf if possible.
    // 2) If 1. is not possible, then we pick a food from overflow shelf which if moved to its matched temperature shelf
    //    would result in the least decrease in value of the food.
    // 3) If none of the food could be moved from overflow shelf to its corresponding temperature shelf, then we discard
    //    the food which is most is most likely to deteriorate faster.
    void AddFood(std::unique_ptr<Food> food);

    // Get the food corresponding to the order. If the food was discarded or the food corresponding to the order was
    // never added to the shelf, then a nullptr is returned.
    std::unique_ptr<Food> GetFood(const Order &order);

private:
    // A private class corresponding to a single row of shelf.
    class SingleRowShelf {
    public:
        SingleRowShelf(const std::string &type, const int capacity) : type_(type), capacity_(capacity) {}

        // Gets the food corresponding to the given order from the row if exists, else return nullptr.
        std::unique_ptr<Food> GetFood(const Order &order);

        // Prevents the row from adding new food items. The food items would be added only after the lock is removed.
        void Lock() {
            mtx_.lock();
        }

        // Start allowing new food items to be added to the row.
        void Unlock() {
            mtx_.unlock();
        }

        // Tries to add the food item to the row if possible. Returns the original food if it is not possible to add the
        // food to the shelf. If the food is successfully added then nullptr is returned. This happens when the row is
        // already full.
        bool TryAdd(std::unique_ptr<Food> food);

        // Should be called only when the row is locked from added new food items to the row. It is used to force add
        // a food item to a row if it is not already full. Usage
        //  row.Lock();
        //  if (row.UnsafeIsFull()) {
        //     row.UnsafeAdd(food);
        //  }
        bool UnsafeAdd(std::unique_ptr<Food> food);

        // Should be called only when the row is locked from adding new food items to the row. This prevents race
        // conditions between different threads adding items to the row. Returns true if currently the row is full.
        bool UnsafeIsFull() {
            return content_.size() == capacity_;
        }

        // Should be called only when the row is locked from adding new food items to the row. Removes the food
        // corresponding to the given order_id.
        std::unique_ptr<Food> UnsafeRemove(const std::string &order_id);

    protected:
        // This unordered map contains the food.
        // key: order_id, value: food.
        std::unordered_map<std::string, std::unique_ptr<Food>> content_;

    private:
        // Temperature type of the shelf.
        std::string type_;
        // Capacity of the shelf;
        const int capacity_;
        // Used to perform lock or unlock on the shelf. See Lock() and Unlock() methods above.
        std::mutex mtx_;
    };

    // OverflowShelf inherits from SingleRowShelf. This class contains additional logic on when food item needs to be
    // removed from its shelf when its capacity limit has reached and a new food item has to be added.
    class OverflowShelf : public SingleRowShelf {
    public:
        OverflowShelf(const std::string &type, const int capacity) : SingleRowShelf(type, capacity) {}

        // Returns the food which needs to be removed from the shelf to make space for new food item.
        // 1) we pick a food from overflow shelf which if moved to its matched temperature shelf would result in the
        //    least decrease in value of the food.
        // 2) If none of the food could be moved from overflow shelf to its corresponding temperature shelf, then we
        //    discard the food which is most is most likely to deteriorate faster.
        std::vector<std::unique_ptr<std::pair<Order, double>>> GetRemovalChoices();

    };

    // Owned Shelves.
    std::unique_ptr<SingleRowShelf> hot_shelf_;
    std::unique_ptr<SingleRowShelf> cold_shelf_;
    std::unique_ptr<SingleRowShelf> frozen_shelf_;
    std::unique_ptr<OverflowShelf> overflow_shelf_;

    // Map of OrderId to the type of the shelf where the food is stored for easy access.
    std::unordered_map<std::string, std::string> order_shelf_map_;
    std::shared_ptr<Logger> logger_;
};


#endif //KITCHENSYNC_SHELVES_H
