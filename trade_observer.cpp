#include "trade_observer.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <ranges>

TradeObserver::TradeObserver(std::string_view host,
                             uint16_t port,
                             uint32_t queue_size)
    : m_queue_(queue_size) {
  std::cout << "connecting to trade observer" << std::endl;
  if (!m_client_sock_.Connect(host, port)) {
    throw std::runtime_error("unable to connect to trade observer");
  }
  std::cout << "connected" << std::endl;
}

bool TradeObserver::Send(std::span<const TradeResult> results) {
  while (m_flag_.test_and_set(std::memory_order_relaxed)) {
  }

  if (results.size() + m_count_ > m_queue_.size()) [[unlikely]] {
    m_flag_.clear();
    return false;
  }

  std::ranges::copy(std::begin(results), std::end(results),
                    std::begin(m_queue_) + m_count_);

  m_count_ += results.size();
  m_flag_.clear();

  return true;
}

void TradeObserver::Run() {
  union {
    const TradeResult* result;
    const uint8_t* buffer;
  } msg;

  msg.result = m_queue_.data();

  while (1) {
    while (m_flag_.test_and_set(std::memory_order_relaxed)) {
      if (m_count_ > 0) {
        const int byte_sent =
            m_client_sock_.Send({msg.buffer, sizeof(TradeResult) * m_count_});

        if (byte_sent <= 0) {
          throw std::runtime_error("unable to send to trade observer");
        }
        m_count_ = 0;
      }

      m_flag_.clear();
    }
  }
}
