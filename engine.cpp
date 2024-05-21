#include "engine.h"
#include <algorithm>
#include <cstring>
#include <ranges>

Engine::Engine(EngineOptions options)
    : kMaxOrders{options.MaxOrderLimit},
      m_allocator_(options.MaxOrderLimit * sizeof(Price_t) * 2 +
                   options.MaxOrderLimit * sizeof(ColdCache) * 2),
      m_buy_count_{0},
      m_sell_count_{0} {
  m_buy_price_caches_ = std::span<Price_t>(
      reinterpret_cast<Price_t*>(m_allocator_.Address()), kMaxOrders);

  m_buy_item_caches_ = std::span<ColdCache>(
      reinterpret_cast<ColdCache*>(m_buy_price_caches_.data() +
                                   m_buy_price_caches_.size()),
      kMaxOrders);

  m_sell_price_caches_ =
      std::span<Price_t>(reinterpret_cast<Price_t*>(m_buy_item_caches_.data() +
                                                    m_buy_item_caches_.size()),
                         kMaxOrders);

  m_sell_item_caches_ = std::span<ColdCache>(
      reinterpret_cast<ColdCache*>(m_sell_price_caches_.data() +
                                   m_sell_price_caches_.size()),
      kMaxOrders);
}

/**
 * 1 lower bound search for the price slot index
 * 2 shift all the elements at the index by 1
 */
void Engine::AddOrder(BuyOrder order) {
  if (m_buy_count_ == m_buy_price_caches_.size()) [[unlikely]] {
    std::cout << "exceeded buy order limit" << std::endl;
    return;
  }

  const Price_t price = order.Price();
  const ID_t id = order.Id();
  const Quantity_t quantity = order.Quantity();

  if (m_buy_count_ == 0) {
    InsertBuyOrderAt(0, price, ColdCache{.Id = id, .Quantity = quantity});
    ++m_buy_count_;
    return;
  }

  ++m_buy_count_;
  // including a new empty slot at the end
  std::span<Price_t> price_slice{m_buy_price_caches_.data(), m_buy_count_};
  std::span<ColdCache> item_slice{m_buy_item_caches_.data(), m_buy_count_};

  auto found_iterator = std::ranges::lower_bound(
      std::begin(price_slice), std::prev(std::end(price_slice)), price,
      [](Price_t lhs, Price_t rhs) { return lhs < rhs; });

  // if not found, this is the highest price, insert at the back
  if (found_iterator == std::prev(std::end(price_slice))) {
    InsertBuyOrderAt(m_buy_count_ - 1, price,
                     ColdCache{.Id = id, .Quantity = quantity});
    return;
  }

  uint32_t index = std::distance(std::begin(price_slice), found_iterator);

  Price_t current_price = price;
  ColdCache current_item{.Id = id, .Quantity = quantity};

  ShiftRightByOneAt(index, m_buy_count_ - index, price_slice, item_slice);

  InsertBuyOrderAt(index, current_price, current_item);
}

void Engine::AddOrder(SellOrder order) {
  if (m_sell_count_ == m_sell_price_caches_.size()) [[unlikely]] {
    std::cout << "exceeded sell order limit" << std::endl;
    return;
  }

  const Price_t price = order.Price();
  const ID_t id = order.Id();
  const Quantity_t quantity = order.Quantity();

  if (m_sell_count_ == 0) {
    InsertSellOrderAt(0, price, ColdCache{.Id = id, .Quantity = quantity});
    ++m_sell_count_;
    return;
  }

  ++m_sell_count_;
  // including a new empty slot at the end
  std::span<Price_t> price_slice{m_sell_price_caches_.data(), m_sell_count_};
  std::span<ColdCache> item_slice{m_sell_item_caches_.data(), m_sell_count_};

  auto found_iterator = std::ranges::lower_bound(
      std::begin(price_slice), std::prev(std::end(price_slice)), price,
      [](Price_t lhs, Price_t rhs) { return lhs > rhs; });

  // if not found, this is the lowest price, insert at the back
  if (found_iterator == std::prev(std::end(price_slice))) {
    InsertSellOrderAt(m_sell_count_ - 1, price,
                      ColdCache{.Id = id, .Quantity = quantity});
    return;
  }

  uint32_t index = std::distance(std::begin(price_slice), found_iterator);

  Price_t current_price = price;
  ColdCache current_item{.Id = id, .Quantity = quantity};

  // shift all the elements to right by 1
  ShiftRightByOneAt(index, m_sell_count_ - index, price_slice, item_slice);

  InsertSellOrderAt(index, current_price, current_item);
}

std::vector<TradeResult> Engine::Execute() {
  std::vector<TradeResult> result;

  int64_t s_i = static_cast<int64_t>(m_sell_count_) - 1;
  int64_t b_i = static_cast<int64_t>(m_buy_count_) - 1;

  while (s_i >= 0 and b_i >= 0 and
         (m_sell_price_caches_[s_i] <= m_buy_price_caches_[b_i])) {
    const Quantity_t quantity = std::min(m_buy_item_caches_[b_i].Quantity,
                                         m_sell_item_caches_[s_i].Quantity);

    const Price_t buy_price = m_buy_price_caches_[b_i];
    const Price_t sell_price = m_sell_price_caches_[s_i];
    const ID_t buy_id = m_buy_item_caches_[b_i].Id;
    const ID_t sell_id = m_sell_item_caches_[s_i].Id;

    m_buy_item_caches_[b_i].Quantity -= quantity;
    m_sell_item_caches_[s_i].Quantity -= quantity;

    result.push_back(TradeResult{.BuyId = buy_id,
                                 .SellId = sell_id,
                                 .BuyPrice = buy_price,
                                 .SellPrice = sell_price,
                                 .Quantity = quantity});

    // move back index by 1 if quantity is depleted

    const uint64_t sell_shift_count = (m_sell_item_caches_[s_i].Quantity == 0);
    const uint64_t buy_shift_count = (m_buy_item_caches_[b_i].Quantity == 0);

    m_sell_count_ -= sell_shift_count;
    m_buy_count_ -= buy_shift_count;

    s_i -= sell_shift_count;
    b_i -= buy_shift_count;
  }

  return result;
}
