#include <pthread.h>
#include <sched.h>
#include <csignal>
#include <cstdint>
#include <iostream>
#include <span>
#include <thread>
#include "engine.h"
#include "heap_based_engine.h"
#include "order_handler.h"
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
    std::cout << "Interrupt Signal: " << strsignal(num) << '\n';
    exit(num);
  };

  action.sa_handler = handler;

  sigaction(SIGINT, &action, nullptr);
}

int main(int, char**) {
  SignalSetup();

  PinCurrentThreadToCore();

  HeapBasedEngine engine;

  TradeObserver trade_observer("127.0.0.1", 8765);

  Server server("127.0.0.1", 5678, OrderHandler{engine, trade_observer});

  std::jthread observer_thread([&trade_observer]() { trade_observer.Run(); });
  server.Run();
}
