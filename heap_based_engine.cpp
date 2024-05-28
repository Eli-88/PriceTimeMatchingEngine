#include "heap_based_engine.h"
#include <algorithm>
#include <ranges>

namespace {
static const auto kBuyComp = std::less{};
static const auto kSellComp = std::greater{};
}  // namespace

HeapBasedEngine::HeapBasedEngine(EngineOptions opts)
    : kMaxOrders{opts.MaxOrderLimit},
      m_allocator_(opts.MaxOrderLimit * sizeof(Key) * 2 +
                   opts.MaxOrderLimit * sizeof(ColdCache) * 2),
      m_buy_price_count_{0},
      m_buy_item_count_{0},
      m_sell_price_count_{0},
      m_sell_item_count_{0} {
  m_buy_price_caches_ = std::span<Key>(
      reinterpret_cast<Key*>(m_allocator_.Address()), kMaxOrders);

  std::ranges::fill(std::begin(m_buy_price_caches_),
                    std::end(m_buy_price_caches_), Key{});

  m_buy_item_caches_ = std::span<ColdCache>(
      reinterpret_cast<ColdCache*>(m_buy_price_caches_.data() +
                                   m_buy_price_caches_.size()),
      kMaxOrders);

  m_sell_price_caches_ =
      std::span<Key>(reinterpret_cast<Key*>(m_buy_item_caches_.data() +
                                            m_buy_item_caches_.size()),
                     kMaxOrders);

  std::ranges::fill(std::begin(m_sell_price_caches_),
                    std::end(m_sell_price_caches_), Key{});

  m_sell_item_caches_ = std::span<ColdCache>(
      reinterpret_cast<ColdCache*>(m_sell_price_caches_.data() +
                                   m_sell_price_caches_.size()),
      kMaxOrders);
}

void HeapBasedEngine::AddOrder(BuyOrder order) noexcept {
  Key& cache = m_buy_price_caches_[m_buy_price_count_++];

  cache.Price = order.Price();
  cache.Sequence = m_sequence_++;

  if (cache.IsIndexValid()) {
    // reuse the item index if valid
    m_buy_item_caches_[cache.Index] =
        ColdCache{.Id = order.Id(), .Quantity = order.Quantity()};
  } else {
    cache.Index = m_buy_item_count_++;
    m_buy_item_caches_[cache.Index] =
        ColdCache{.Id = order.Id(), .Quantity = order.Quantity()};
  }

  std::ranges::push_heap(std::begin(m_buy_price_caches_),
                         std::begin(m_buy_price_caches_) + m_buy_price_count_,
                         kBuyComp);
}

void HeapBasedEngine::AddOrder(SellOrder order) noexcept {
  Key& cache = m_sell_price_caches_[m_sell_price_count_++];

  cache.Price = order.Price();
  cache.Sequence = m_sequence_++;

  if (cache.IsIndexValid()) {
    // reuse the item index if valid
    m_sell_item_caches_[cache.Index] =
        ColdCache{.Id = order.Id(), .Quantity = order.Quantity()};
  } else {
    cache.Index = m_sell_item_count_++;
    m_sell_item_caches_[cache.Index] =
        ColdCache{.Id = order.Id(), .Quantity = order.Quantity()};
  }

  std::ranges::push_heap(std::begin(m_sell_price_caches_),
                         std::begin(m_sell_price_caches_) + m_sell_price_count_,
                         kSellComp);
}

std::vector<TradeResult> HeapBasedEngine::Execute() noexcept {
  std::vector<TradeResult> results;

  Key buy_price;
  Key sell_price;

  std::ranges::pop_heap(std::begin(m_buy_price_caches_),
                        std::begin(m_buy_price_caches_) + m_buy_price_count_,
                        kBuyComp);

  std::ranges::pop_heap(std::begin(m_sell_price_caches_),
                        std::begin(m_sell_price_caches_) + m_sell_price_count_,
                        kSellComp);

  while (m_buy_price_count_ > 0 and m_sell_price_count_ > 0) {
    const uint32_t b_i = m_buy_price_count_ - 1;
    const uint32_t s_i = m_sell_price_count_ - 1;

    const Price_t buy_price = m_buy_price_caches_[b_i].Price;
    const Price_t sell_price = m_sell_price_caches_[s_i].Price;

    if (buy_price < sell_price) {
      break;
    }

    const uint32_t buy_index = m_buy_price_caches_[b_i].Index;
    const uint32_t sell_index = m_sell_price_caches_[s_i].Index;

    auto& buy_item = m_buy_item_caches_[buy_index];
    auto& sell_item = m_sell_item_caches_[sell_index];

    const Quantity_t min_quantity =
        std::min(buy_item.Quantity, sell_item.Quantity);

    results.emplace_back(TradeResult{.BuyId = buy_item.Id,
                                     .SellId = sell_item.Id,
                                     .BuyPrice = buy_price,
                                     .SellPrice = sell_price,
                                     .Quantity = min_quantity});

    buy_item.Quantity -= min_quantity;
    sell_item.Quantity -= min_quantity;

    if (buy_item.Quantity == 0) {
      --m_buy_price_count_;

      std::ranges::pop_heap(
          std::begin(m_buy_price_caches_),
          std::begin(m_buy_price_caches_) + m_buy_price_count_, kBuyComp);
    }

    if (sell_item.Quantity == 0) {
      --m_sell_price_count_;

      std::ranges::pop_heap(
          std::begin(m_sell_price_caches_),
          std::begin(m_sell_price_caches_) + m_sell_price_count_, kSellComp);
    }
  }

  // adjust back the last element for push heap

  if (m_buy_price_count_ > 0) {
    std::ranges::push_heap(std::begin(m_buy_price_caches_),
                           std::begin(m_buy_price_caches_) + m_buy_price_count_,
                           kBuyComp);
  }

  if (m_sell_price_count_ > 0) {
    std::ranges::push_heap(
        std::begin(m_sell_price_caches_),
        std::begin(m_sell_price_caches_) + m_sell_price_count_, kSellComp);
  }

  return results;
}
