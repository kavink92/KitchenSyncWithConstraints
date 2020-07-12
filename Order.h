//
// Created by kavin on 6/27/20.
//

#ifndef KITCHENSYNC_ORDER_H
#define KITCHENSYNC_ORDER_H


#include <string>

class Order {
public:
    Order(const std::string &id, const std::string &name, const std::string &temp, const int shelf_life,
          const double decay_rate) : id_(id), name_(name), temp_(temp), shelf_life_(shelf_life),
                                     decay_rate_(decay_rate) {}

    // Getter functions for Order related data.
    std::string Id() const { return id_; }

    std::string Temp() const { return temp_; }

    int Shelflife() const { return shelf_life_; }

    double DecayRate() const { return decay_rate_; }

    // Prints the data encapsulated by this Order object.
    void Debug() const;

private:
    const std::string id_;
    const std::string name_;
    const std::string temp_;
    const int shelf_life_;
    const double decay_rate_;
};


#endif //KITCHENSYNC_ORDER_H
