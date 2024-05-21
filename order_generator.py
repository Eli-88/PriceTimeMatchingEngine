import random
import sys


def generate_orders(num_orders):
    orders = []
    unique_id = 1
    for _ in range(num_orders):
        order_type = random.choice(["B", "S"])
        price = random.randint(1, 255)
        quantity = random.randint(1, 65535)
        order = f"{order_type},{unique_id},{price},{quantity}\n"
        orders.append(order)
        unique_id += 1
    return orders


def write_orders_to_file(file_path, orders):
    with open(file_path, "w") as f:
        f.writelines(orders)


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python script.py <num_orders> <output_file>")
        sys.exit(1)

    num_orders = int(sys.argv[1])
    output_file = sys.argv[2]

    orders = generate_orders(num_orders)
    write_orders_to_file(output_file, orders)
    print(f"{num_orders} orders generated and written to {output_file}")
