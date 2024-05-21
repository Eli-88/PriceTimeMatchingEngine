#pragma once

#include <cstdint>

class MemoryLock {
 public:
  MemoryLock(void* address, uint32_t len);
  ~MemoryLock();

  MemoryLock(const MemoryLock&) = delete;
  MemoryLock& operator=(const MemoryLock&) = delete;

 private:
  void* m_address_;
  uint32_t m_len_;
};
