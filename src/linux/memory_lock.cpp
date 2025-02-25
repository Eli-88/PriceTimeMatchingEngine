#include "linux/memory_lock.h"
#include <sys/mman.h>
#include "error.h"

MemoryLock::MemoryLock(void* address, uint32_t len)
    : m_address_{address}, m_len_{len} {
  const int err = ::mlock(address, len);
  if (err == -1) {
    throw MmapLockError();
  }
}

MemoryLock::~MemoryLock() {
  ::munlock(m_address_, m_len_);
}
