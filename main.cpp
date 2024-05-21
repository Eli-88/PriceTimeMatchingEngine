#include <pthread.h>
#include <sched.h>
#include <csignal>
#include <cstdint>
#include <iostream>
#include <span>
#include <thread>
#include "engine.h"
#include "server.h"
#include "trade_observer.h"

static void PinCurrentThreadToCore() {
  const int core_id = sched_getcpu();

  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_id, &cpuset);
  pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
}

void SignalSetup() {
  struct sigaction action;
  std::memset(&action, 0, sizeof(action));

  auto handler = +[](int num) {
    std::cout << "Interrupt Signal: " << strsignal(num) << std::endl;
    exit(num);
  };

  action.sa_handler = handler;

  sigaction(SIGINT, &action, nullptr);
  sigaction(SIGPIPE, &action, nullptr);
}

int main(int, char**) {
  SignalSetup();

  PinCurrentThreadToCore();

  Engine engine;

  TradeObserver trade_observer("127.0.0.1", 8765);

  Server server("127.0.0.1", 5678,
                [&engine, &trade_observer](std::span<const uint8_t> buffer) {
                  assert(buffer.size() >= sizeof(Order));
                  assert(buffer.size() % sizeof(Order) == 0);

                  union {
                    const uint8_t* raw;
                    const Order* order;
                    const BuyOrder* buy_order;
                    const SellOrder* sell_order;
                  } msg;

                  msg.raw = buffer.data();
                  int msg_count = buffer.size() / sizeof(Order);

                  for (int i = 0; i < msg_count; ++i) {
                    switch (msg.order[i].OrderType()) {
                      case kBuy:
                        engine.AddOrder(msg.buy_order[i]);
                        break;
                      case kSell:
                        engine.AddOrder(msg.sell_order[i]);
                        break;
                      [[unlikely]] default:
                        break;
                    }
                    const std::vector<TradeResult> trade_results =
                        engine.Execute();

                    if (!trade_results.empty()) {
                      trade_observer.Send(trade_results);
                    }
                  }
                });

  std::jthread observer_thread([&trade_observer]() { trade_observer.Run(); });
  server.Run();
}
