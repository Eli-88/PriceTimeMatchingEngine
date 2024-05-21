#include "server.h"
#include <cassert>
#include <cstring>
#include <iostream>

Server::Server(std::string host, uint16_t port, RecvCallBack recv_callback)
    : m_recv_callback_{recv_callback} {
  SetSocketReusable(m_server_fd_);
  SetSocketNoDelay(m_server_fd_);

  m_server_fd_.Bind(host, port);
  m_server_fd_.Listen(5);
}

void Server::Run() {
  while (1) {
    TcpSocket conn = m_server_fd_.Accept();

    constexpr int kBufLen{8192};
    uint8_t buffer[kBufLen];
    int offset = 0;

    while (1) {
      const int byte_recv =
          conn.Recv(
              {buffer + offset, static_cast<uint64_t>(kBufLen - offset)}) +
          offset;

      if (byte_recv <= 0) [[unlikely]] {
        break;
      }

      const int trancated_len = byte_recv % sizeof(Order);

      if (byte_recv >= static_cast<int>(sizeof(Order))) [[likely]] {
        m_recv_callback_(
            {buffer, static_cast<uint64_t>(byte_recv - trancated_len)});

        if (trancated_len > 0) {
          std::memcpy(buffer, buffer + (byte_recv - trancated_len),
                      trancated_len);
        }
      }

      offset = trancated_len;
    }
  }
}
