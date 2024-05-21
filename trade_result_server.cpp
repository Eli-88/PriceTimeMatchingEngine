#include <array>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>
#include "tcp.h"
#include "trade_result.h"

int main() {
  TcpSocket server_fd;

  SetSocketReusable(server_fd);
  SetSocketNoDelay(server_fd);

  server_fd.Bind("127.0.0.1", 8765);
  server_fd.Listen(5);

  std::array<uint8_t, 8192> buffer;

  std::vector<TradeResult> all_results;

  union {
    const uint8_t* buf;
    const TradeResult* result;
  } msg;

  msg.buf = buffer.data();

  int offset = 0;

  while (1) {
    TcpSocket conn = server_fd.Accept();

    while (1) {
      const int byte_recv =
          conn.Recv({buffer.data() + offset, buffer.size() - offset}) + offset;

      if (byte_recv <= 0) {
        break;
      }

      const int count = byte_recv / sizeof(TradeResult);
      const int trancated_len = byte_recv % sizeof(TradeResult);

      for (int i = 0; i < count; ++i) {
        const TradeResult& result = msg.result[i];

        std::printf(
            "buy id: %ld, sell id: %ld, buy price: %d, sell price: %d, "
            "quantity: "
            "%d\n",
            result.BuyId, result.SellId, result.BuyPrice, result.SellPrice,
            result.Quantity);
      }

      std::memcpy(buffer.data(), buffer.data() + (byte_recv - trancated_len),
                  trancated_len);
      offset = trancated_len;
    }
  }
}
