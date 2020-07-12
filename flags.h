//
// Created by kavin on 7/3/20.
//

#ifndef KITCHENSYNC_FLAGS_H
#define KITCHENSYNC_FLAGS_H

namespace flags {

    // The min time after which the courier would pick up the food.
    static constexpr int kMinPickupTime = 2;
    // The max time before which the courier would pick up the food.
    static constexpr int kMaxPickupTime = 6;
    // Ingestion rate of the order generator (orders/sec)
    static constexpr double kOrderIngestionRate = 20000;
    // Json path from which order generator generates orders from.
    static constexpr const char *kOrdersJsonPath = "../orders.json";
}

#endif //KITCHENSYNC_FLAGS_H
