//
// Created by kavin on 6/27/20.
//

#include <fstream>
#include <iostream>
#include "OrderGenerator.h"
#include "flags.h"

using json = nlohmann::json;
using namespace std::chrono;


namespace {
    double IngestionRate(const system_clock::time_point &start, const int num_completed_orders) {
        std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - start;
        auto ingestion_rate = static_cast<double>(num_completed_orders) / elapsed_seconds.count();
        return ingestion_rate;
    }

    std::unique_ptr<Food>
    GetFoodFromShelf(const Order &order, std::shared_ptr<Shelves> shelves, std::shared_ptr<Logger> logger) {
        std::this_thread::sleep_for(std::chrono::seconds(4));

        //logger->Log("Typing to receive order: " + order.Id());
        auto food = shelves->GetFood(order);
        if (food == nullptr) {
            logger->Log("Could not get order: " + order.Id());
        } else {
            food->PickedUp();
            auto time2 = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            logger->Log("Food with Id: " + order.Id() + " was picked up at " + std::ctime(&time2) + " with value " +
                        std::to_string(food->ValueNow()));

            // TODO: Deliver Food.
        }
    }
}

OrderGenerator::OrderGenerator(std::shared_ptr<Shelves> shelves, std::shared_ptr<KitchenOrderQueue> kod,
                               std::shared_ptr<CourierOrderQueue> cod, std::shared_ptr<Logger> logger) : shelves_(
        shelves), kod_(kod), cod_(cod), logger_(logger) {
}

void OrderGenerator::Generate() {
    std::ifstream i(flags::kOrdersJsonPath);
    json json_file;
    i >> json_file;

    // Trying something
    std::vector<std::future<void>> add_promise;
    std::vector<std::future<std::unique_ptr<Food>>> food_promise;

    auto start = std::chrono::system_clock::now();
    int num_added = 0;
    for (const auto &j : json_file) {
        auto order = std::make_unique<Order>(j.at("id"), j.at("name"), j.at("temp"), j.at("shelfLife"),
                                             j.at("decayRate"));
        auto courier_order = std::make_unique<Order>(*order);
        std::string order_id = order->Id();
        auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        logger_->Log("Order Id: " + order_id + " generated at time: " + std::ctime(&time));

        if (num_added == 0) {
            add_promise.push_back(std::async(&Shelves::AddFood, shelves_, std::make_unique<Food>(*order)));
            //kod_->AddOrder(std::move(order));


        } else if (IngestionRate(start, num_added + 1) > flags::kOrderIngestionRate) {
            std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - start;
            double sleep_time =
                    static_cast<double>(num_added + 1) / flags::kOrderIngestionRate - elapsed_seconds.count();
            long int sleep_time_milli = static_cast<long int>(sleep_time * 1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time_milli));
            //kod_->AddOrder(std::move(order));

            add_promise.push_back(std::async(&Shelves::AddFood, shelves_, std::make_unique<Food>(*order)));
        } else {
            //kod_->AddOrder(std::move(order));

            add_promise.push_back(std::async(&Shelves::AddFood, shelves_, std::make_unique<Food>(*order)));
        }
        num_added++;

        //**********************************

        //food_promise.push_back(std::async(&GetFoodFromShelf, *courier_order, shelves_, logger_));
        food_promise.push_back(std::async(&Shelves::GetFood, shelves_, *courier_order));
        // **************************************

        //cod_->AddOrder(std::move(courier_order));

    }

    logger_->Log("Num added: " + std::to_string(num_added));
    std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - start;
    logger_->Log("Time for processing: " + std::to_string(elapsed_seconds.count()) + "s");

    double net_value = 0;
    for (int i = 0; i < food_promise.size(); i++) {
        add_promise[i].get();
        auto food = food_promise[i].get();
        if (food != nullptr) {
            if (food->WasPickedUp()) {
                net_value += food->ValueAtPickup();
            }
        }

    }

    std::cout << "Net value: " << std::to_string(net_value) << std::endl;
    logger_->Log("Net value: " + std::to_string(net_value));
}
