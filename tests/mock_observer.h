#pragma once

#include <gmock/gmock.h>
#include <vector>
#include "trade_result.h"

class MockObserver {
 public:
  MOCK_METHOD(bool, Send, (std::span<const TradeResult>), ());
};
