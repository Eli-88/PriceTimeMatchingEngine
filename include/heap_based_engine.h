#pragma once

#include <cstdint>
#include <limits>
#include <span>
#include "define.h"
#include "engine_options.h"
#include "order_handler.h"

class HeapBasedEngine {
 private:
  struct Item {
    uint32_t Sequence{0};  // to maintain the time priority
                           // for identical price
    Price_t Price;
    ID_t Id;
    Quantity_t Quantity;

    bool operator<(const Item& rhs) const {
      if (Price == rhs.Price) {
        return Sequence > rhs.Sequence;
      }

      return Price < rhs.Price;
    }

    bool operator>(const Item& rhs) const {
      if (Price == rhs.Price) {
        return Sequence > rhs.Sequence;
      }

      return Price > rhs.Price;
    }
  } __attribute__((packed, aligned(1)));

  static const uint32_t kMaxOrders = 1 << 18;

 public:
  HeapBasedEngine();

  void AddOrder(BuyOrder order) noexcept;
  void AddOrder(SellOrder order) noexcept;

  std::vector<TradeResult> Execute() noexcept;

 private:
  uint8_t m_caches_[kMaxOrders * sizeof(Item) * 2];

  std::span<HeapBasedEngine::Item> m_buy_caches_;
  uint32_t m_buy_count_{0};

  std::span<HeapBasedEngine::Item> m_sell_caches_;
  uint32_t m_sell_count_{0};

  uint32_t m_sequence_{0};
};
