#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
#include <span>
#include <vector>
#include "define.h"
#include "engine_options.h"
#include "order.h"
#include "trade_result.h"

class Engine {
 private:
  struct ColdCache {
    ID_t Id;
    Quantity_t Quantity;
  } __attribute__((packed, aligned(1)));

  static const uint32_t kMaxOrders = 1 << 18;

 public:
  Engine();

  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;
  void AddOrder(BuyOrder order) noexcept;
  void AddOrder(SellOrder order) noexcept;

  std::vector<TradeResult> Execute() noexcept;

 private:
  __attribute__((always_inline)) void InsertBuyOrderAt(
      uint32_t index,
      Price_t price,
      Engine::ColdCache item) noexcept {
    m_buy_price_caches_[index] = price;
    m_buy_item_caches_[index] = item;
  }

  __attribute__((always_inline)) void InsertSellOrderAt(
      uint32_t index,
      Price_t price,
      Engine::ColdCache item) noexcept {
    m_sell_price_caches_[index] = price;
    m_sell_item_caches_[index] = item;
  }

  __attribute__((always_inline)) void ShiftRightByOneAt(
      uint32_t index,
      uint32_t len,
      auto&& price_slice,
      auto&& item_slice) noexcept {
    std::memmove(price_slice.data() + index + 1, price_slice.data() + index,
                 len * sizeof(Price_t));

    std::memmove(item_slice.data() + index + 1, item_slice.data() + index,
                 len * sizeof(ColdCache));
  }

 private:
  uint8_t m_caches_[kMaxOrders * sizeof(Price_t) * 2 +
                    kMaxOrders * sizeof(ColdCache) * 2];

  std::span<Price_t> m_buy_price_caches_;
  std::span<Engine::ColdCache> m_buy_item_caches_;
  uint32_t m_buy_count_;

  std::span<Price_t> m_sell_price_caches_;
  std::span<Engine::ColdCache> m_sell_item_caches_;
  uint32_t m_sell_count_;
};
