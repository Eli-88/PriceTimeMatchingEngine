#pragma once

#include "define.h"

class Order {
 public:
  Order(OrderType_t order_type, ID_t id, Price_t price, Quantity_t quantity)
      : m_order_type_{order_type},
        m_id_{id},
        m_price_{price},
        m_quantity_{quantity} {}

  ID_t Id() const { return m_id_; }
  Price_t Price() const { return m_price_; }
  Quantity_t Quantity() const { return m_quantity_; }
  OrderType_t OrderType() const { return m_order_type_; }

 private:
  OrderType_t m_order_type_;
  ID_t m_id_;
  Price_t m_price_;
  Quantity_t m_quantity_;
} __attribute__((packed, aligned(1)));

class BuyOrder : public Order {
 public:
  BuyOrder(ID_t id, Price_t price, Quantity_t quantity)
      : Order(kBuy, id, price, quantity) {}
} __attribute__((packed, aligned(1)));

class SellOrder : public Order {
 public:
  SellOrder(ID_t id, Price_t price, Quantity_t quantity)
      : Order(kSell, id, price, quantity) {}
} __attribute__((packed, aligned(1)));
