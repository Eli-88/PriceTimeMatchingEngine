#include "tcp.h"
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include "error.h"

TcpSocket::TcpSocket() {
  m_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);

  if (m_fd_ == -1) {
    throw TcpSocketCreateError();
  }
}

TcpSocket::TcpSocket(int fd) : m_fd_{fd} {}

bool TcpSocket::Connect(std::string_view host, unsigned short port) {
  sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));

  addr.sin_addr.s_addr = ::inet_addr(host.data());
  addr.sin_port = ::htons(port);
  addr.sin_family = AF_INET;

  return -1 != ::connect(m_fd_, reinterpret_cast<const sockaddr*>(&addr),
                         sizeof(addr));
}

void TcpSocket::Bind(std::string_view host, unsigned short port) {
  sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));

  addr.sin_addr.s_addr = ::inet_addr(host.data());
  addr.sin_port = ::htons(port);
  addr.sin_family = AF_INET;

  int err =
      ::bind(m_fd_, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));

  if (err == -1) {
    throw TcpBindError();
  }
}

void TcpSocket::Listen(int backlog) {
  int err = ::listen(m_fd_, backlog);

  if (err == -1) {
    throw TcpListenError();
  }
}

TcpSocket TcpSocket::Accept() {
  sockaddr_in addr;
  socklen_t addr_len = sizeof(addr);

  const int new_conn =
      ::accept(m_fd_, reinterpret_cast<sockaddr*>(&addr), &addr_len);

  if (new_conn == -1) {
    throw TcpAcceptError();
  }

  SetSocketNoDelay(new_conn);

  return TcpSocket(new_conn);
}

int TcpSocket::Recv(std::span<uint8_t> buffer) {
  return ::recv(m_fd_, buffer.data(), buffer.size(), 0);
}

int TcpSocket::Send(std::span<const uint8_t> buffer) {
  return ::send(m_fd_, buffer.data(), buffer.size(), MSG_NOSIGNAL);
}

void TcpSocket::Close() {
  ::shutdown(m_fd_, SHUT_RDWR);
  ::close(m_fd_);
}

void SetSocketReusable(TcpSocket sock) {
  const int yes = 1;
  if (-1 ==
      ::setsockopt(sock.Fd(), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) {
    throw TcpSocketOptReuseAddrError();
  }
}

void SetSocketNoDelay(TcpSocket sock) {
  const int yes = 1;
  if (-1 ==
      ::setsockopt(sock.Fd(), IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes))) {
    throw TcpSocketOptNoDelayError();
  }
}

bool IsNoDelay(TcpSocket sock) {
  int flag;
  socklen_t len = sizeof(flag);

  if (getsockopt(sock.Fd(), IPPROTO_TCP, TCP_NODELAY, &flag, &len) > -1) {
    return flag > 0;
  }

  return false;
}
