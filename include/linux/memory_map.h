#pragma once

#include <sys/mman.h>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <span>
#include "error.h"

template <typename T>
class Mmap {
 public:
  Mmap(uint32_t len) {
    void* address = ::mmap(nullptr, len, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);

    if (address == MAP_FAILED) {
      throw MmapMapFailError();
    }

    m_address_ = reinterpret_cast<T*>(address);
    m_len_ = len / sizeof(T);
  }

  Mmap(const Mmap&) = delete;
  Mmap& operator=(const Mmap&) = delete;

  ~Mmap() { ::munmap(m_address_, m_len_); }

  uint32_t Len() const { return m_len_; }
  T& operator[](uint32_t index) {
    assert(index < m_len_);
    return m_address_[index];
  }

  T* Address() const { return m_address_; }

 private:
  T* m_address_;
  uint32_t m_len_;
};
