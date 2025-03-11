#include <gtest/gtest.h>
#include <memory>
#include "heap_based_engine.h"
#include "order.h"

TEST(HeapBasedEngineTest, SimpleBuyOrdersAddAndExecute) {
  auto engine = std::make_unique<HeapBasedEngine>();

  engine->AddOrder(BuyOrder(ID_t{1}, Price_t{20}, Quantity_t{12}));
  engine->AddOrder(BuyOrder(ID_t{2}, Price_t{14}, Quantity_t{1}));
  engine->AddOrder(BuyOrder(ID_t{3}, Price_t{64}, Quantity_t{90}));
  engine->AddOrder(BuyOrder(ID_t{4}, Price_t{63}, Quantity_t{54}));
  engine->AddOrder(BuyOrder(ID_t{5}, Price_t{0}, Quantity_t{190}));

  engine->AddOrder(SellOrder(ID_t{6}, Price_t{30}, Quantity_t{10}));
  engine->AddOrder(SellOrder(ID_t{7}, Price_t{40}, Quantity_t{200}));

  auto trade_results = engine->Execute();
  EXPECT_EQ(trade_results.size(), 3);

  EXPECT_EQ(trade_results[0].BuyId, ID_t{3});
  EXPECT_EQ(trade_results[0].SellId, ID_t{6});
  EXPECT_EQ(trade_results[0].Quantity, Quantity_t{10});
  EXPECT_EQ(trade_results[0].BuyPrice, Price_t{64});
  EXPECT_EQ(trade_results[0].SellPrice, Price_t{30});

  EXPECT_EQ(trade_results[1].BuyId, ID_t{3});
  EXPECT_EQ(trade_results[1].SellId, ID_t{7});
  EXPECT_EQ(trade_results[1].Quantity, Quantity_t{80});
  EXPECT_EQ(trade_results[1].BuyPrice, Price_t{64});
  EXPECT_EQ(trade_results[1].SellPrice, Price_t{40});

  EXPECT_EQ(trade_results[2].BuyId, ID_t{4});
  EXPECT_EQ(trade_results[2].SellId, ID_t{7});
  EXPECT_EQ(trade_results[2].Quantity, Quantity_t{54});
  EXPECT_EQ(trade_results[2].BuyPrice, Price_t{63});
  EXPECT_EQ(trade_results[2].SellPrice, Price_t{40});
}

TEST(HeapBasedEngineTest, BuyOrdersMatchAtEqualPrices) {
  auto engine = std::make_unique<HeapBasedEngine>();

  engine->AddOrder(BuyOrder(ID_t{1}, Price_t{20}, Quantity_t{10}));
  engine->AddOrder(BuyOrder(ID_t{2}, Price_t{20}, Quantity_t{15}));
  engine->AddOrder(BuyOrder(ID_t{3}, Price_t{20}, Quantity_t{20}));

  engine->AddOrder(SellOrder(ID_t{4}, Price_t{20}, Quantity_t{30}));

  auto trade_results = engine->Execute();
  EXPECT_EQ(trade_results.size(), 3);

  // Ensure buy orders match with the sell order at equal prices
  EXPECT_EQ(trade_results[0].BuyId, ID_t{1});
  EXPECT_EQ(trade_results[0].SellId, ID_t{4});
  EXPECT_EQ(trade_results[0].Quantity, Quantity_t{10});

  EXPECT_EQ(trade_results[1].BuyId, ID_t{2});
  EXPECT_EQ(trade_results[1].SellId, ID_t{4});
  EXPECT_EQ(trade_results[1].Quantity, Quantity_t{15});

  EXPECT_EQ(trade_results[2].BuyId, ID_t{3});
  EXPECT_EQ(trade_results[2].SellId, ID_t{4});
  EXPECT_EQ(trade_results[2].Quantity, Quantity_t{5});
}

TEST(HeapBasedEngineTest, SellOrdersMatchAtEqualPrices) {
  auto engine = std::make_unique<HeapBasedEngine>();

  engine->AddOrder(SellOrder(ID_t{1}, Price_t{30}, Quantity_t{10}));
  engine->AddOrder(SellOrder(ID_t{2}, Price_t{30}, Quantity_t{15}));
  engine->AddOrder(SellOrder(ID_t{3}, Price_t{30}, Quantity_t{20}));

  engine->AddOrder(BuyOrder(ID_t{4}, Price_t{30}, Quantity_t{30}));

  auto trade_results = engine->Execute();
  EXPECT_EQ(trade_results.size(), 3);

  // Ensure sell orders match with the buy order at equal prices
  EXPECT_EQ(trade_results[0].BuyId, ID_t{4});
  EXPECT_EQ(trade_results[0].SellId, ID_t{1});
  EXPECT_EQ(trade_results[0].Quantity, Quantity_t{10});

  EXPECT_EQ(trade_results[1].BuyId, ID_t{4});
  EXPECT_EQ(trade_results[1].SellId, ID_t{2});
  EXPECT_EQ(trade_results[1].Quantity, Quantity_t{15});

  EXPECT_EQ(trade_results[2].BuyId, ID_t{4});
  EXPECT_EQ(trade_results[2].SellId, ID_t{3});
  EXPECT_EQ(trade_results[2].Quantity, Quantity_t{5});
}

TEST(HeapBasedEngineTest, PartialFillOrders) {
  auto engine = std::make_unique<HeapBasedEngine>();

  engine->AddOrder(BuyOrder(ID_t{1}, Price_t{30}, Quantity_t{20}));
  engine->AddOrder(SellOrder(ID_t{2}, Price_t{30}, Quantity_t{15}));

  auto trade_results = engine->Execute();
  EXPECT_EQ(trade_results.size(), 1);

  // Ensure partial fill of the buy order
  EXPECT_EQ(trade_results[0].BuyId, ID_t{1});
  EXPECT_EQ(trade_results[0].SellId, ID_t{2});
  EXPECT_EQ(trade_results[0].Quantity, Quantity_t{15});
}

TEST(HeapBasedEngineTest, NoMatchOrders) {
  auto engine = std::make_unique<HeapBasedEngine>();

  engine->AddOrder(BuyOrder(ID_t{1}, Price_t{30}, Quantity_t{20}));
  engine->AddOrder(SellOrder(ID_t{2}, Price_t{40}, Quantity_t{15}));

  auto trade_results = engine->Execute();
  EXPECT_EQ(trade_results.size(), 0);

  // Ensure no trades executed when there's no match
}

TEST(HeapBasedEngineTest, SamePriceDifferentTime) {
  auto engine = std::make_unique<HeapBasedEngine>();

  engine->AddOrder(BuyOrder(ID_t{1}, Price_t{30}, Quantity_t{20}));
  engine->AddOrder(SellOrder(ID_t{2}, Price_t{30}, Quantity_t{10}));
  engine->AddOrder(SellOrder(ID_t{3}, Price_t{30}, Quantity_t{15}));

  auto trade_results = engine->Execute();
  EXPECT_EQ(trade_results.size(), 2);

  // Ensure trades are executed based on time priority at the same price
  EXPECT_EQ(trade_results[0].BuyId, ID_t{1});
  EXPECT_EQ(trade_results[0].SellId, ID_t{2});
  EXPECT_EQ(trade_results[0].Quantity, Quantity_t{10});

  EXPECT_EQ(trade_results[1].BuyId, ID_t{1});
  EXPECT_EQ(trade_results[1].SellId, ID_t{3});
  EXPECT_EQ(trade_results[1].Quantity, Quantity_t{10});
}

TEST(HeapBasedEngineTest, EmptyOrderBook) {
  auto engine = std::make_unique<HeapBasedEngine>();

  auto trade_results = engine->Execute();
  EXPECT_EQ(trade_results.size(), 0);

  // Ensure no trades are executed when the order book is empty
}

TEST(HeapBasedEngineTest, SingleOrderPartialFill) {
  auto engine = std::make_unique<HeapBasedEngine>();

  engine->AddOrder(BuyOrder(ID_t{1}, Price_t{30}, Quantity_t{20}));
  engine->AddOrder(SellOrder(ID_t{2}, Price_t{30}, Quantity_t{30}));

  auto trade_results = engine->Execute();
  EXPECT_EQ(trade_results.size(), 1);

  // Ensure partial fill of the sell order
  EXPECT_EQ(trade_results[0].BuyId, ID_t{1});
  EXPECT_EQ(trade_results[0].SellId, ID_t{2});
  EXPECT_EQ(trade_results[0].Quantity, Quantity_t{20});
}

TEST(HeapBasedEngineTest, OrderBookWithSamePriceDifferentType) {
  auto engine = std::make_unique<HeapBasedEngine>();

  engine->AddOrder(BuyOrder(ID_t{1}, Price_t{30}, Quantity_t{20}));
  engine->AddOrder(SellOrder(ID_t{2}, Price_t{30}, Quantity_t{30}));
  engine->AddOrder(BuyOrder(ID_t{3}, Price_t{30}, Quantity_t{10}));

  auto trade_results = engine->Execute();
  EXPECT_EQ(trade_results.size(), 2);

  // Ensure trades are executed correctly with orders of the same price but
  // different types
  EXPECT_EQ(trade_results[0].BuyId, ID_t{1});
  EXPECT_EQ(trade_results[0].SellId, ID_t{2});
  EXPECT_EQ(trade_results[0].Quantity, Quantity_t{20});

  EXPECT_EQ(trade_results[1].BuyId, ID_t{3});
  EXPECT_EQ(trade_results[1].SellId, ID_t{2});
  EXPECT_EQ(trade_results[1].Quantity, Quantity_t{10});
}

TEST(HeapBasedEngineTest, SingleOrderFullFill) {
  auto engine = std::make_unique<HeapBasedEngine>();

  engine->AddOrder(BuyOrder(ID_t{1}, Price_t{30}, Quantity_t{20}));
  engine->AddOrder(SellOrder(ID_t{2}, Price_t{30}, Quantity_t{20}));

  auto trade_results = engine->Execute();
  EXPECT_EQ(trade_results.size(), 1);

  // Ensure full fill of the buy order
  EXPECT_EQ(trade_results[0].BuyId, ID_t{1});
  EXPECT_EQ(trade_results[0].SellId, ID_t{2});
  EXPECT_EQ(trade_results[0].Quantity, Quantity_t{20});
}

TEST(HeapBasedEngineTest, MultipleOrdersWithSamePrice) {
  auto engine = std::make_unique<HeapBasedEngine>();

  engine->AddOrder(BuyOrder(ID_t{1}, Price_t{30}, Quantity_t{20}));
  engine->AddOrder(BuyOrder(ID_t{2}, Price_t{30}, Quantity_t{30}));
  engine->AddOrder(SellOrder(ID_t{3}, Price_t{30}, Quantity_t{50}));

  auto trade_results = engine->Execute();
  EXPECT_EQ(trade_results.size(), 2);

  // Ensure trades are executed correctly with multiple orders at the same price
  EXPECT_EQ(trade_results[0].BuyId, ID_t{1});
  EXPECT_EQ(trade_results[0].SellId, ID_t{3});
  EXPECT_EQ(trade_results[0].Quantity, Quantity_t{20});

  EXPECT_EQ(trade_results[1].BuyId, ID_t{2});
  EXPECT_EQ(trade_results[1].SellId, ID_t{3});
  EXPECT_EQ(trade_results[1].Quantity, Quantity_t{30});
}

TEST(HeapBasedEngineTest, NoBuyOrders) {
  auto engine = std::make_unique<HeapBasedEngine>();

  engine->AddOrder(SellOrder(ID_t{1}, Price_t{30}, Quantity_t{20}));
  engine->AddOrder(SellOrder(ID_t{2}, Price_t{40}, Quantity_t{30}));

  auto trade_results = engine->Execute();
  EXPECT_EQ(trade_results.size(), 0);

  // Ensure no trades are executed when there are no buy orders
}

TEST(HeapBasedEngineTest, NoSellOrders) {
  auto engine = std::make_unique<HeapBasedEngine>();

  engine->AddOrder(BuyOrder(ID_t{1}, Price_t{30}, Quantity_t{20}));
  engine->AddOrder(BuyOrder(ID_t{2}, Price_t{40}, Quantity_t{30}));

  auto trade_results = engine->Execute();
  EXPECT_EQ(trade_results.size(), 0);

  // Ensure no trades are executed when there are no sell orders
}

TEST(HeapBasedEngineTest, MatchingWithDifferentQuantity) {
  auto engine = std::make_unique<HeapBasedEngine>();

  engine->AddOrder(BuyOrder(ID_t{1}, Price_t{30}, Quantity_t{20}));
  engine->AddOrder(SellOrder(ID_t{2}, Price_t{30}, Quantity_t{15}));

  auto trade_results = engine->Execute();
  EXPECT_EQ(trade_results.size(), 1);

  // Ensure partial fill of the buy order due to different quantities
  EXPECT_EQ(trade_results[0].BuyId, ID_t{1});
  EXPECT_EQ(trade_results[0].SellId, ID_t{2});
  EXPECT_EQ(trade_results[0].Quantity, Quantity_t{15});
}

TEST(HeapBasedEngineTest, MatchingWithMultipleTrades) {
  auto engine = std::make_unique<HeapBasedEngine>();

  engine->AddOrder(BuyOrder(ID_t{1}, Price_t{30}, Quantity_t{20}));
  engine->AddOrder(BuyOrder(ID_t{2}, Price_t{30}, Quantity_t{30}));
  engine->AddOrder(SellOrder(ID_t{3}, Price_t{30}, Quantity_t{50}));

  auto trade_results = engine->Execute();
  EXPECT_EQ(trade_results.size(), 2);

  // Ensure multiple trades are executed with different buy orders
  EXPECT_EQ(trade_results[0].BuyId, ID_t{1});
  EXPECT_EQ(trade_results[0].SellId, ID_t{3});
  EXPECT_EQ(trade_results[0].Quantity, Quantity_t{20});

  EXPECT_EQ(trade_results[1].BuyId, ID_t{2});
  EXPECT_EQ(trade_results[1].SellId, ID_t{3});
  EXPECT_EQ(trade_results[1].Quantity, Quantity_t{30});
}

TEST(HeapBasedEngineTest, NoMatchingOrders) {
  auto engine = std::make_unique<HeapBasedEngine>();

  engine->AddOrder(BuyOrder(ID_t{1}, Price_t{30}, Quantity_t{20}));
  engine->AddOrder(SellOrder(ID_t{2}, Price_t{40}, Quantity_t{30}));

  auto trade_results = engine->Execute();
  EXPECT_EQ(trade_results.size(), 0);

  // Ensure no trades are executed when there are no matching orders
}

TEST(HeapBasedEngineTest, MatchingWithSameQuantity) {
  auto engine = std::make_unique<HeapBasedEngine>();

  engine->AddOrder(BuyOrder(ID_t{1}, Price_t{30}, Quantity_t{20}));
  engine->AddOrder(SellOrder(ID_t{2}, Price_t{30}, Quantity_t{20}));

  auto trade_results = engine->Execute();
  EXPECT_EQ(trade_results.size(), 1);

  // Ensure full fill of the buy order with matching sell order
  EXPECT_EQ(trade_results[0].BuyId, ID_t{1});
  EXPECT_EQ(trade_results[0].SellId, ID_t{2});
  EXPECT_EQ(trade_results[0].Quantity, Quantity_t{20});
}

TEST(HeapBasedEngineTest, NoBuyOrSellOrders) {
  auto engine = std::make_unique<HeapBasedEngine>();

  auto trade_results = engine->Execute();
  EXPECT_EQ(trade_results.size(), 0);

  // Ensure no trades are executed when there are no buy or sell orders
}

TEST(HeapBasedEngineTest, SingleOrderWithZeroQuantity) {
  auto engine = std::make_unique<HeapBasedEngine>();

  engine->AddOrder(BuyOrder(ID_t{1}, Price_t{30}, Quantity_t{0}));

  auto trade_results = engine->Execute();
  EXPECT_EQ(trade_results.size(), 0);

  // Ensure no trades are executed when an order has zero quantity
}

TEST(HeapBasedEngineTest, LargeQuantityOrders) {
  auto engine = std::make_unique<HeapBasedEngine>();

  // Add a large quantity buy order and sell order
  engine->AddOrder(BuyOrder(ID_t{1}, Price_t{30}, Quantity_t{10000}));
  engine->AddOrder(SellOrder(ID_t{2}, Price_t{30}, Quantity_t{10000}));

  auto trade_results = engine->Execute();
  EXPECT_EQ(trade_results.size(), 1);

  // Ensure full fill of the buy order with matching sell order
  EXPECT_EQ(trade_results[0].BuyId, ID_t{1});
  EXPECT_EQ(trade_results[0].SellId, ID_t{2});
  EXPECT_EQ(trade_results[0].Quantity, Quantity_t{10000});
}
