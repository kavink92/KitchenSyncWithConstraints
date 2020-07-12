#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <future>
#include <chrono>
#include "OrderGenerator.h"
#include "Kitchen.h"
#include "CourierOrderQueue.h"
#include "Logger.h"

int main() {
    auto logger = std::make_shared<Logger>();
    auto shelves = std::make_shared<Shelves>(logger);

    auto courier_order_queue = std::make_shared<CourierOrderQueue>();
    auto kitchen_order_queue = std::make_shared<KitchenOrderQueue>();
    CourierExecutor courier_executor(shelves, courier_order_queue, logger);
    Kitchen kitchen(shelves, kitchen_order_queue, logger);

    // Order generator which generates the orders.
    OrderGenerator order_generator(kitchen_order_queue, courier_order_queue, logger);
    std::thread order_generator_thread(&OrderGenerator::Generate, order_generator);

    // Imposing a constraint on the number of couriers and chefs in kitchen.
    int num_threads = 100;
    std::thread kitchen_threads[num_threads];
    std::thread courier_threads[num_threads];
    for (int i = 0; i < num_threads; i++) {
        kitchen_threads[i] = std::thread(&Kitchen::Run, kitchen);
        courier_threads[i] = std::thread(&CourierExecutor::Run, courier_executor);
    }
    for (int i = 0; i < num_threads; i++) {
        kitchen_threads[i].join();
        courier_threads[i].join();
    }

    // All the threads are joined to the main thread and hence the program exits only after all the child threads
    // finish.
    order_generator_thread.join();
    logger->Log("Simulation is over, let's go home");
}
