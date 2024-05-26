#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "order.h"
#include "tcp.h"

void generate_order(const char* file_path,
                    std::vector<uint8_t>& buffer,
                    uint32_t max_order) {
  std::ifstream file(file_path);
  assert(file.is_open());

  std::string line;

  union {
    BuyOrder* buy;
    SellOrder* sell;
    uint8_t* data;
  } buf;

  buf.data = buffer.data();

  uint32_t index = 0;

  while (std::getline(file, line) && index < max_order) {
    std::string item;
    std::stringstream ss(line);
    std::string order_type;

    std::getline(ss, order_type, ',');

    std::getline(ss, item, ',');
    const ID_t id = std::stoi(item);

    std::getline(ss, item, ',');
    const Price_t price = std::stoi(item);

    std::getline(ss, item, ',');
    const Quantity_t quantity = std::stoi(item);

    if (order_type == "B") {
      buf.buy[index] = BuyOrder(id, price, quantity);

    } else {
      buf.sell[index] = SellOrder(id, price, quantity);
    }

    ++index;
  }

  std::cout << "finish loading!" << '\n';
  file.close();

  TcpSocket conn;
  SetSocketReusable(conn);
  SetSocketNoDelay(conn);

  bool success = conn.Connect("127.0.0.1", 5678);
  if (!success) {
    std::cerr << "unable to connect" << '\n';
    exit(-1);
  }

  std::cout << "sending orders to matching engine..." << index << '\n';

  conn.Send({buf.data, index * sizeof(Order)});
}

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << "<file_path> <max order>\n";
    exit(-1);
  }

  const char* file_path = argv[1];
  uint64_t max_order = std::stoul(argv[2]);

  std::cout << "max order: " << max_order << '\n';

  std::vector<uint8_t> buffer;
  buffer.resize(max_order * sizeof(Order));

  generate_order(file_path, buffer, max_order);
}
