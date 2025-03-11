#pragma once

#include <cassert>
#include <concepts>
#include <cstdint>
#include <span>
#include "engine_interface.h"
#include "observer_interface.h"
#include "trade_result.h"

template <Engine_t Engine, Observer_t Observer>
class OrderHandler {
 public:
  OrderHandler(Engine& engine, Observer& observer)
      : m_engine_{engine}, m_observer_{observer} {}

  void operator()(std::span<const uint8_t> buffer) {
    assert(buffer.size() >= sizeof(Order));
    assert(buffer.size() % sizeof(Order) == 0);

    union {
      const uint8_t* raw;
      const Order* order;
      const BuyOrder* buy_order;
      const SellOrder* sell_order;
    } msg;

    msg.raw = buffer.data();
    int msg_count = buffer.size() / sizeof(Order);

    for (int i = 0; i < msg_count; ++i) {
      switch (msg.order[i].OrderType()) {
        case kBuy:
          m_engine_.AddOrder(msg.buy_order[i]);
          break;
        case kSell:
          m_engine_.AddOrder(msg.sell_order[i]);
          break;
        [[unlikely]] default:
          break;
      }
      const std::vector<TradeResult> trade_results = m_engine_.Execute();

      if (!trade_results.empty()) {
        // keep trying until succeed
        while (!m_observer_.Send(trade_results)) {
        }
      }
    }
  }

 private:
  Engine& m_engine_;
  Observer& m_observer_;
};
