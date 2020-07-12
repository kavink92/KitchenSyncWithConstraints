cmake_minimum_required(VERSION 3.16)
project(KitchenSync)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -pthread)
add_executable(KitchenSync main.cpp OrderGenerator.cpp OrderGenerator.h Order.cpp Order.h Kitchen.cpp Kitchen.h Food.cpp Food.h Shelves.cpp Shelves.h CourierExecutor.cpp CourierExecutor.h CourierOrderQueue.cpp CourierOrderQueue.h KitchenOrderQueue.cpp KitchenOrderQueue.h flags.h Logger.cpp Logger.h)

