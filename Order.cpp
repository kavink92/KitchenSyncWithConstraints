//
// Created by kavin on 6/27/20.
//

#include <iostream>
#include "Order.h"

void Order::Debug() const {
    std::cout << "Id: " << id_ << std::endl;
    std::cout << "Name: " << name_ << std::endl;
    std::cout << "Temp: " << temp_ << std::endl;
    std::cout << "Shelf Life: " << shelf_life_ << std::endl;
    std::cout << "Decay Rate: " << decay_rate_ << std::endl;
}
