# Price Time Priority Order Matching Engine Implementation

This is a sample demo implementation of low latency price time order matching engine in C++


- **Source Code**: 
   - It comprises of 3 different cpp source main file and 1 python script
        - `main.cpp` -> matching engine
        -  `data_generator.cpp` -> load orders from file and sending order to matching engine
        - `trade_result_server.cpp` -> to observe trade results
        - `order_generator.py` -> to generate random orders for data generator to load

- **Compiling Instruction**:
     - `git clone --recurse-submodules https://github.com/Eli-88/PriceTimeMatchingEngine.git`
     - `mkdir build`
     - `cd build`
     - `cmake -DCMAKE_BUILD_TYPE=Release ..`
     - `make -j8`

- **Running Instructions**
     - check the size of the huge page to be around 2MB: `cat /proc/meminfo | grep Hugepagesize`
     - ensure enough huge page for mmap: `sudo sysctl -w vm.nr_hugepages=10` 
     - navigate to root folder
     - generate 500000 random orders for data generator: `python order_generator.py 500000 order_input.txt`
     - run trade results server first: `./build/trade_result_server`
     - run matching engine next: `./build/matching_engine`
     - run to load order file and send matching engine:  `./build/data_generator order_input.txt 500000`
     - should be able to see trade results in trade result server

- **TCP Format Specifications**:
  - Orders
    - `Order Type` (uint8_t) -> `Buy 0x00, Sell 0x01`
    - `Unique Id` (uint64_t)
    - `Price` (uint16_t)
    - `Quantity` (uint16_t)
  - Trade Result
    - `Buy Id` (uint64_t)
    - `Sell Id` (uint64_t)
    - `Buy Price` (uint16_t)
    - `Sell Price` (uint16_t)
    - `Quantity` (uint16_t)
    
- **Test**:
    - All test case can be found in the unit test under `tests/test_engine.cpp`
    - run unit test: `./build/test_matching_engine`
   
- **Matching Engine Specification**
    - Cache Spec
        - 6 cores in all
        - single core spec
            - L1d cache: 288 KiB
            - L1i cache: 192 KiB 
            - L2 cache: 7.5 MiB

    - Max order limits that may be held at any time
        - Buy Orders: up to 262144
        - Sell Orders: up to 262144
        - Take up to 6MB, which fit tightly into the L2 cache
    - OS
        - tested on ubuntu 22.04