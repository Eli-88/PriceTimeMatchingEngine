cmake_minimum_required(VERSION 3.22)

project(MatchingEngine)

set(CMAKE_CXX_STANDARD 23)

set(SRC
    error.cpp
    engine.cpp
    memory_lock.cpp
    tcp.cpp
    server.cpp
    trade_observer.cpp)

include_directories(.)

add_executable(matching_engine main.cpp)
target_sources(matching_engine PRIVATE ${SRC})

target_compile_options(matching_engine PRIVATE -Wall -Werror)
add_subdirectory(googletest)

add_executable(test_matching_engine
    tests/main.cpp)

target_sources(test_matching_engine PRIVATE
    ${SRC}
    tests/test_engine.cpp
    tests/test_order_handler.cpp)

target_link_libraries(test_matching_engine PRIVATE
    gtest
    gtest_main
    gmock
    gmock_main)

enable_testing()

add_executable(data_generator data_generator.cpp)
target_sources(data_generator PRIVATE ${SRC})

add_executable(trade_result_server trade_result_server.cpp)
target_sources(trade_result_server PRIVATE ${SRC})
