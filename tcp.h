#pragma once

#include <cstdint>
#include <span>
#include <string_view>

class TcpSocket {
 public:
  TcpSocket();
  TcpSocket(int fd);
  bool operator==(int fd) const { return m_fd_ == fd; }

  bool Connect(std::string_view host, unsigned short port);

  void Bind(std::string_view host, unsigned short port);
  void Listen(int backlog);

  TcpSocket Accept();
  int Recv(std::span<uint8_t> buffer);
  int Send(std::span<const uint8_t> buffer);

  void Close();

  const int Fd() { return m_fd_; }

 private:
  int m_fd_;
};

void SetSocketReusable(TcpSocket sock);
void SetSocketNoDelay(TcpSocket sock);
bool IsNoDelay(TcpSocket sock);