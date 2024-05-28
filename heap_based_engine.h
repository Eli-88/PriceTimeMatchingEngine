#pragma once

#include <cstdint>
#include <limits>
#include <span>
#include "define.h"
#include "engine_options.h"
#include "memory_map.h"
#include "order_handler.h"

class HeapBasedEngine {
 private:
  struct ColdCache {
    ID_t Id;
    Quantity_t Quantity;
  } __attribute__((packed, aligned(1)));

  struct Key {
    uint32_t Sequence{
        std::numeric_limits<uint32_t>::max()};  // to maintain the time priority
                                                // for identical price
    uint32_t Index{
        std::numeric_limits<uint32_t>::max()};  // cold cache index, so we can
                                                // reuse the index if this key
                                                // is removed
    Price_t Price;

    bool IsIndexValid() const {
      return Index != std::numeric_limits<uint32_t>::max();
    }

    bool operator<(const Key& rhs) const {
      if (Price == rhs.Price) {
        return Sequence > rhs.Sequence;
      }

      return Price < rhs.Price;
    }

    bool operator>(const Key& rhs) const {
      if (Price == rhs.Price) {
        return Sequence > rhs.Sequence;
      }

      return Price > rhs.Price;
    }
  } __attribute__((packed, aligned(1)));

 public:
  HeapBasedEngine(EngineOptions options = {.MaxOrderLimit = 1 << 18});

  void AddOrder(BuyOrder order) noexcept;
  void AddOrder(SellOrder order) noexcept;

  std::vector<TradeResult> Execute() noexcept;

 private:
  const uint32_t kMaxOrders;
  Mmap<uint8_t> m_allocator_;

  std::span<HeapBasedEngine::Key> m_buy_price_caches_;
  std::span<HeapBasedEngine::ColdCache> m_buy_item_caches_;
  uint32_t m_buy_price_count_{0};
  uint32_t m_buy_item_count_{0};

  std::span<HeapBasedEngine::Key> m_sell_price_caches_;
  std::span<HeapBasedEngine::ColdCache> m_sell_item_caches_;
  uint32_t m_sell_price_count_{0};
  uint32_t m_sell_item_count_{0};

  uint32_t m_sequence_{0};
};
