add_library(linux)

target_sources(linux PRIVATE
    memory_lock.cpp
    tcp.cpp)