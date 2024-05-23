#pragma once

#include <gmock/gmock.h>
#include <vector>
#include "order.h"
#include "trade_result.h"

class MockEngine {
 public:
  MOCK_METHOD(void, AddOrder, (const BuyOrder&), ());
  MOCK_METHOD(void, AddOrder, (const SellOrder&), ());
  MOCK_METHOD(std::vector<TradeResult>, Execute, (), ());
};
