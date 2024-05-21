#pragma once

#include <atomic>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>
#include "tcp.h"
#include "trade_result.h"

class TradeObserver {
 public:
  TradeObserver(std::string_view host,
                uint16_t port,
                uint32_t queue_size = 10'000);
  TradeObserver(const TradeObserver&) = delete;
  TradeObserver& operator=(const TradeObserver&) = delete;

  void Send(std::span<const TradeResult> results);
  void Run();

 private:
  std::vector<TradeResult> m_queue_;
  uint32_t m_count_{0};
  std::atomic_flag m_flag_{ATOMIC_FLAG_INIT};

  TcpSocket m_client_sock_;
};
