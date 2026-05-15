cmake_minimum_required(VERSION 3.10)
project(CTC_singleOrderBook)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(orderbook main.cpp OrderBook.cpp)