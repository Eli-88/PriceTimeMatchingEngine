cmake_minimum_required(VERSION 3.22)

project(MatchingEngine)

set(CMAKE_CXX_STANDARD 23)

include_directories(include)

add_subdirectory(googletest)
add_subdirectory(src)
add_subdirectory(tests)