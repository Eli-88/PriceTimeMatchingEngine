add_executable(test_matching_engine
    main.cpp)

target_include_directories(test_matching_engine PRIVATE .)
    
target_sources(test_matching_engine PRIVATE
    ${SRC}
    test_engine.cpp
    test_order_handler.cpp
    test_heap_base_engine.cpp)

target_compile_options(test_matching_engine PRIVATE -fsanitize=address -fno-omit-frame-pointer)

target_link_libraries(test_matching_engine PRIVATE
    matching_engine_lib
    gtest
    gtest_main
    gmock
    gmock_main
    asan)