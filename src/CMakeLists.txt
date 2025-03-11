set(SRC
    linux/tcp.cpp
    linux/memory_lock.cpp
    error.cpp
    engine.cpp
    server.cpp
    trade_observer.cpp
    heap_based_engine.cpp)

include_directories(.)

add_library(matching_engine_lib)
target_sources(matching_engine_lib PRIVATE ${SRC})
target_compile_options(matching_engine_lib PRIVATE -Wall -Werror)


add_executable(matching_engine main.cpp)
target_link_libraries(matching_engine PRIVATE matching_engine_lib)
target_compile_options(matching_engine PRIVATE -Werror=frame-larger-than=1048576)

add_executable(data_generator data_generator.cpp)
target_sources(data_generator PRIVATE ${SRC})
target_link_libraries(data_generator PRIVATE matching_engine_lib)

add_executable(trade_result_server trade_result_server.cpp)
target_sources(trade_result_server PRIVATE ${SRC})
target_link_libraries(trade_result_server PRIVATE matching_engine_lib)
