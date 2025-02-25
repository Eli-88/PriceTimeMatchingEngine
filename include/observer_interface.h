#pragma once

#include <concepts>
#include <cstdint>
#include <span>
#include "trade_result.h"

template <class T>
concept Observer_t =
    requires(T observer, std::span<const TradeResult> results) {
      { observer.Send(results) } -> std::same_as<bool>;
    };
