//
// Created by kavin on 7/3/20.
//

#ifndef KITCHENSYNC_FLAGS_H
#define KITCHENSYNC_FLAGS_H

namespace flags {

    static constexpr int kMinPickupTime = 2;
    static constexpr int kMaxPickupTime = 6;
    static constexpr double kOrderIngestionRate = 20000;

    static constexpr const char *kOrdersJsonPath = "../orders.json";
}

#endif //KITCHENSYNC_FLAGS_H
