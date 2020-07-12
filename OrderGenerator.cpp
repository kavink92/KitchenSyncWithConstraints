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
    // Gets the current ingestion rate of the order generator.
    double IngestionRate(const system_clock::time_point &start, const int num_completed_orders) {
        std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - start;
        auto ingestion_rate = static_cast<double>(num_completed_orders) / elapsed_seconds.count();
        return ingestion_rate;
    }
}  // namespace

OrderGenerator::OrderGenerator(std::shared_ptr<KitchenOrderQueue> kitchen_order_queue,
                               std::shared_ptr<CourierOrderQueue> courier_order_queue, std::shared_ptr<Logger> logger)
        : kitchen_order_queue_(kitchen_order_queue), courier_order_queue_(courier_order_queue), logger_(logger) {
}

void OrderGenerator::Generate() {
    // Reads from JSON file.
    std::ifstream i(flags::kOrdersJsonPath);
    json json_file;
    i >> json_file;

    auto start = std::chrono::system_clock::now();
    int num_added = 0;
    for (const auto &record : json_file) {
        auto order = std::make_unique<Order>(record.at("id"), record.at("name"), record.at("temp"),
                                             record.at("shelfLife"),
                                             record.at("decayRate"));
        auto courier_order = std::make_unique<Order>(*order);
        std::string order_id = order->Id();
        auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        logger_->Log("Order Id: " + order_id + " generated at time: " + std::ctime(&time));

        if (num_added == 0) {
            kitchen_order_queue_->AddOrder(std::move(order));
        } else if (IngestionRate(start, num_added + 1) > flags::kOrderIngestionRate) {
            std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - start;
            double sleep_time =
                    static_cast<double>(num_added + 1) / flags::kOrderIngestionRate - elapsed_seconds.count();
            long int sleep_time_milli = static_cast<long int>(sleep_time * 1000);

            // Sleep for sleep_time_milli ms in order to match the ingestion rate.
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time_milli));
            kitchen_order_queue_->AddOrder(std::move(order));
        } else {
            kitchen_order_queue_->AddOrder(std::move(order));
        }
        num_added++;
        courier_order_queue_->AddOrder(std::move(courier_order));
    }
}
