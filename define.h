#pragma once

#include <cstdint>

using ID_t = uint64_t;
using Price_t = uint16_t;
using OrderType_t = uint8_t;
using Quantity_t = uint16_t;

constexpr OrderType_t kBuy = 0;
constexpr OrderType_t kSell = 1;
