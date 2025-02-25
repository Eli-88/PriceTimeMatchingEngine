#pragma once

#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <vector>
#include "linux/tcp.h"
#include "order.h"

class Server {
 public:
  using RecvCallBack = std::function<void(std::span<const uint8_t>)>;

 public:
  Server(std::string host, uint16_t port, RecvCallBack recv_callback);

  void Run();  // this is blocking run

 private:
  RecvCallBack m_recv_callback_;

  TcpSocket m_server_fd_;
};
