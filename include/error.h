#pragma once

#include <exception>
#include <string>

class BaseIOError : public std::exception {
 public:
  BaseIOError(std::string error_msg);
  const char* what() const noexcept override;

 private:
  mutable std::string m_error_msg_;
};

class MmapMapFailError : public BaseIOError {
 public:
  MmapMapFailError() : BaseIOError("mmap map fail error") {}
};

class MmapLockError : public BaseIOError {
 public:
  MmapLockError() : BaseIOError("mmap lock fail error") {}
};

class TcpSocketCreateError : public BaseIOError {
 public:
  TcpSocketCreateError() : BaseIOError("tcp socket create error") {}
};

class TcpSocketOptReuseAddrError : public BaseIOError {
 public:
  TcpSocketOptReuseAddrError()
      : BaseIOError("tcp setsockopt reuse addr error") {}
};

class TcpSocketOptNoDelayError : public BaseIOError {
 public:
  TcpSocketOptNoDelayError() : BaseIOError("tcp setsockopt no delay error") {}
};

class TcpBindError : public BaseIOError {
 public:
  TcpBindError() : BaseIOError("tcp bind error") {}
};

class TcpListenError : public BaseIOError {
 public:
  TcpListenError() : BaseIOError("tcp listen error") {}
};

class TcpAcceptError : public BaseIOError {
 public:
  TcpAcceptError() : BaseIOError("tcp accept error") {}
};
