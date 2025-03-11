#include "heap_based_engine.h"
#include <algorithm>
#include <iostream>
#include <ranges>

namespace {
static const auto kBuyComp = std::less{};
static const auto kSellComp = std::greater{};
}  // namespace

HeapBasedEngine::HeapBasedEngine() : m_buy_count_{0}, m_sell_count_{0} {
  m_buy_caches_ =
      std::span<Item>(reinterpret_cast<Item*>(m_caches_), kMaxOrders);

  std::ranges::fill(std::begin(m_buy_caches_), std::end(m_buy_caches_), Item{});

  m_sell_caches_ =
      std::span<Item>(m_buy_caches_.data() + m_buy_caches_.size(), kMaxOrders);

  std::ranges::fill(std::begin(m_sell_caches_), std::end(m_sell_caches_),
                    Item{});
}

void HeapBasedEngine::AddOrder(BuyOrder order) noexcept {
  if (m_buy_count_ == m_buy_caches_.size()) [[unlikely]] {
    std::cout << "exceeded buy order limit" << '\n';
    return;
  }

  Item& cache = m_buy_caches_[m_buy_count_++];

  cache.Price = order.Price();
  cache.Sequence = m_sequence_++;
  cache.Id = order.Id();
  cache.Quantity = order.Quantity();

  std::ranges::push_heap(std::begin(m_buy_caches_),
                         std::begin(m_buy_caches_) + m_buy_count_, kBuyComp);
}

void HeapBasedEngine::AddOrder(SellOrder order) noexcept {
  if (m_sell_count_ == m_sell_caches_.size()) [[unlikely]] {
    std::cout << "exceeded sell order limit" << '\n';
    return;
  }

  Item& cache = m_sell_caches_[m_sell_count_++];

  cache.Price = order.Price();
  cache.Sequence = m_sequence_++;
  cache.Id = order.Id();
  cache.Quantity = order.Quantity();

  std::ranges::push_heap(std::begin(m_sell_caches_),
                         std::begin(m_sell_caches_) + m_sell_count_, kSellComp);
}

std::vector<TradeResult> HeapBasedEngine::Execute() noexcept {
  std::vector<TradeResult> results;

  std::ranges::pop_heap(std::begin(m_buy_caches_),
                        std::begin(m_buy_caches_) + m_buy_count_, kBuyComp);

  std::ranges::pop_heap(std::begin(m_sell_caches_),
                        std::begin(m_sell_caches_) + m_sell_count_, kSellComp);

  while (m_buy_count_ > 0 and m_sell_count_ > 0) {
    const uint32_t b_i = m_buy_count_ - 1;
    const uint32_t s_i = m_sell_count_ - 1;

    auto& buy = m_buy_caches_[b_i];
    auto& sell = m_sell_caches_[s_i];

    if (buy.Price < sell.Price) {
      break;
    }

    const Quantity_t min_quantity = std::min(buy.Quantity, sell.Quantity);

    results.emplace_back(TradeResult{.BuyId = buy.Id,
                                     .SellId = sell.Id,
                                     .BuyPrice = buy.Price,
                                     .SellPrice = sell.Price,
                                     .Quantity = min_quantity});

    buy.Quantity -= min_quantity;
    sell.Quantity -= min_quantity;

    if (buy.Quantity == 0) {
      --m_buy_count_;

      std::ranges::pop_heap(std::begin(m_buy_caches_),
                            std::begin(m_buy_caches_) + m_buy_count_, kBuyComp);
    }

    if (sell.Quantity == 0) {
      --m_sell_count_;

      std::ranges::pop_heap(std::begin(m_sell_caches_),
                            std::begin(m_sell_caches_) + m_sell_count_,
                            kSellComp);
    }
  }

  // adjust back the last element for push heap

  if (m_buy_count_ > 0) {
    std::ranges::push_heap(std::begin(m_buy_caches_),
                           std::begin(m_buy_caches_) + m_buy_count_, kBuyComp);
  }

  if (m_sell_count_ > 0) {
    std::ranges::push_heap(std::begin(m_sell_caches_),
                           std::begin(m_sell_caches_) + m_sell_count_,
                           kSellComp);
  }

  return results;
}
