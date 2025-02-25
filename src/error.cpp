#include "error.h"
#include <errno.h>
#include <cstring>

BaseIOError::BaseIOError(std::string error_msg)
    : m_error_msg_{std::move(error_msg)} {}

const char* BaseIOError::what() const noexcept {
  m_error_msg_.append(": ");
  m_error_msg_.append(::strerror(errno));
  return m_error_msg_.data();
}
