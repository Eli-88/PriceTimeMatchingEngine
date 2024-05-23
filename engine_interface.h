#pragma once

#include <concepts>
#include <vector>
#include "order.h"
#include "trade_result.h"

template <class T>
concept Engine_t =
    requires(T engine, BuyOrder buy_order, SellOrder sell_order) {
      { engine.AddOrder(buy_order) } -> std::same_as<void>;
      { engine.AddOrder(sell_order) } -> std::same_as<void>;
      { engine.Execute() } -> std::same_as<std::vector<TradeResult>>;
    };
