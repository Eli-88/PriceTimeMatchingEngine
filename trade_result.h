#pragma once

#include "define.h"

struct TradeResult {
  ID_t BuyId;
  ID_t SellId;
  Price_t BuyPrice;
  Price_t SellPrice;
  Quantity_t Quantity;
} __attribute__((packed, aligned(1)));