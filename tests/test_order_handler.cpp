#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "mock_engine.h"
#include "mock_observer.h"
#include "order.h"
#include "order_handler.h"
#include "trade_result.h"

using ::testing::_;

bool operator==(const BuyOrder& a, const BuyOrder& b) {
  return a.Id() == b.Id() and a.OrderType() == b.OrderType() and
         a.Price() == b.Price() and a.Quantity() == b.Quantity();
}

bool operator==(const SellOrder& a, const SellOrder& b) {
  return a.Id() == b.Id() and a.OrderType() == b.OrderType() and
         a.Price() == b.Price() and a.Quantity() == b.Quantity();
}

bool operator==(const TradeResult& a, const TradeResult& b) {
  return a.BuyId == b.BuyId and a.SellId == b.SellId and
         a.BuyPrice == b.BuyPrice and a.SellPrice == b.SellPrice and
         a.Quantity == b.Quantity;
}

bool operator==(std::span<const TradeResult> a,
                std::span<const TradeResult> b) {
  return std::equal(std::begin(a), std::end(a), std::begin(b), std::end(b));
}

TEST(EngineTest, SingleBuyOrderExecuteOnce) {
  const BuyOrder buy_order{ID_t{1}, Price_t{100}, Quantity_t{40}};
  std::vector<TradeResult> trade_results;

  MockEngine mock_engine;
  MockObserver mock_observer;

  EXPECT_CALL(mock_engine, AddOrder(buy_order)).Times(1);

  EXPECT_CALL(mock_engine, AddOrder(::testing::A<const SellOrder&>())).Times(0);
  EXPECT_CALL(mock_engine, Execute())
      .Times(1)
      .WillOnce(::testing::Return(trade_results));
  EXPECT_CALL(mock_observer, Send(_)).Times(0);

  OrderHandler handler(mock_engine, mock_observer);

  union {
    const BuyOrder* order;
    const uint8_t* data;
  } msg;

  msg.order = &buy_order;
  std::span<const uint8_t> buffer{msg.data, sizeof(buy_order)};

  handler(buffer);
}

TEST(EngineTest, SingleSellOrderExecuteOnce) {
  const SellOrder sell_order{ID_t{1}, Price_t{100}, Quantity_t{40}};
  std::vector<TradeResult> trade_results;

  MockEngine mock_engine;
  MockObserver mock_observer;

  EXPECT_CALL(mock_engine, AddOrder(::testing::A<const BuyOrder&>())).Times(0);

  EXPECT_CALL(mock_engine, AddOrder(sell_order)).Times(1);
  EXPECT_CALL(mock_engine, Execute())
      .Times(1)
      .WillOnce(::testing::Return(trade_results));
  EXPECT_CALL(mock_observer, Send(_)).Times(0);

  OrderHandler handler(mock_engine, mock_observer);

  union {
    const SellOrder* order;
    const uint8_t* data;
  } msg;

  msg.order = &sell_order;
  std::span<const uint8_t> buffer{msg.data, sizeof(sell_order)};

  handler(buffer);
}

TEST(EngineTest, SingleSellOrderSingleBuyOrderExecuteTwice) {
  const SellOrder sell_order{ID_t{1}, Price_t{100}, Quantity_t{40}};
  const BuyOrder buy_order{ID_t{2}, Price_t{54}, Quantity_t{7}};

  std::vector<TradeResult> trade_results;

  MockEngine mock_engine;
  MockObserver mock_observer;

  EXPECT_CALL(mock_engine, AddOrder(buy_order)).Times(1);

  EXPECT_CALL(mock_engine, AddOrder(sell_order)).Times(1);
  EXPECT_CALL(mock_engine, Execute())
      .Times(2)
      .WillOnce(::testing::Return(trade_results));
  EXPECT_CALL(mock_observer, Send(_)).Times(0);

  OrderHandler handler(mock_engine, mock_observer);

  union {
    SellOrder* sell;
    BuyOrder* buy;
    uint8_t* data;
  } msg;

  constexpr int kBufSize = sizeof(BuyOrder) + sizeof(SellOrder);
  uint8_t data[kBufSize];
  msg.data = data;

  msg.buy[0] = buy_order;
  msg.sell[1] = sell_order;

  std::span<const uint8_t> buffer{msg.data, kBufSize};

  handler(buffer);
}

TEST(EngineTest, TradeResultObserved) {
  const SellOrder sell_order{ID_t{1}, Price_t{100}, Quantity_t{40}};
  std::vector<TradeResult> trade_results;

  trade_results.push_back(TradeResult{.BuyId = 0,
                                      .SellId = 1,
                                      .BuyPrice = 43,
                                      .SellPrice = 15,
                                      .Quantity = 76});

  MockEngine mock_engine;
  MockObserver mock_observer;

  EXPECT_CALL(mock_engine, AddOrder(::testing::A<const BuyOrder&>())).Times(0);

  EXPECT_CALL(mock_engine, AddOrder(sell_order)).Times(1);
  EXPECT_CALL(mock_engine, Execute())
      .Times(1)
      .WillOnce(::testing::Return(trade_results));

  std::span<const TradeResult> trade_buffer{trade_results.data(),
                                            trade_results.size()};

  EXPECT_CALL(mock_observer, Send(trade_buffer))
      .Times(1)
      .WillOnce(::testing::Return(true));

  OrderHandler handler(mock_engine, mock_observer);

  union {
    const SellOrder* order;
    const uint8_t* data;
  } msg;

  msg.order = &sell_order;
  std::span<const uint8_t> buffer{msg.data, sizeof(sell_order)};

  handler(buffer);
}
