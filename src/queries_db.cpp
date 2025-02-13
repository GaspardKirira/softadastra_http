// src/database/queries/ProductQueries.hpp
#pragma once
#include <string>
#include <memory>
#include "config/Config.hpp"

namespace ProductQueries
{
    const std::string INSERT = "INSERT INTO products (name, price) VALUES (?, ?)";
    const std::string UPDATE = "UPDATE products SET name = ?, price = ? WHERE id = ?";
    const std::string DELETE = "DELETE FROM products WHERE id = ?";
    const std::string GET_BY_ID = "SELECT id, name, price FROM products WHERE id = ?";
    const std::string GET_ALL = "SELECT id, name, price FROM products";
}

namespace UserQueries
{
    const std::string INSERT = "INSERT INTO users (username, email, password) VALUES (?, ?, ?)";
    const std::string UPDATE = "UPDATE users SET username = ?, email = ?, password = ? WHERE id = ?";
    const std::string DELETE = "DELETE FROM users WHERE id = ?";
    const std::string GET_BY_ID = "SELECT id, username, email FROM users WHERE id = ?";
    const std::string GET_ALL = "SELECT id, username, email FROM users";
}

namespace OrderQueries
{
    const std::string INSERT = "INSERT INTO orders (user_id, total_price, status) VALUES (?, ?, ?)";
    const std::string UPDATE_STATUS = "UPDATE orders SET status = ? WHERE id = ?";
    const std::string DELETE = "DELETE FROM orders WHERE id = ?";
    const std::string GET_BY_ID = "SELECT id, user_id, total_price, status FROM orders WHERE id = ?";
    const std::string GET_ALL = "SELECT id, user_id, total_price, status FROM orders";
}

class Product
{
public:
    std::string name{};
    double price{};
};

class ProductRepository
{
public:
    void insert(const Product &product);
};

void ProductRepository::insert(const Product &product)
{
    Config &config = Config::getInstance();
    config.loadConfig();
    auto con = config.getDbConnection();
    std::unique_ptr<sql::PreparedStatement> stmt(
        con->prepareStatement(ProductQueries::INSERT));
    stmt->setString(1, product.name);
    stmt->setDouble(2, product.price);
    stmt->executeUpdate();
}
