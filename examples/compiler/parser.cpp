#include "sql-compiler/Lexer.hpp"
#include "sql-compiler/Parser.hpp"
#include <iostream>

using namespace DB;

void test_query(const char* description, string sql) {
    std::cout << "\n========================================\n";
    std::cout << "Test: " << description << "\n";
    std::cout << "SQL: " << sql << "\n";
    std::cout << "========================================\n";

    try {
        RANodePtr ra = compile_sql(sql);
        ra_tree_print(ra);
        std::cout << "✓ Success\n";
    } catch (const std::exception& e) {
        std::cout << "✗ Error: " << e.what() << "\n";
    }
}

int main() {
    std::cout << "=== SQL Compiler Test Suite ===\n";

    // Basic SELECT
    test_query("Simple SELECT *",
        "SELECT * FROM users;");

    test_query("SELECT with columns",
        "SELECT id, name, email FROM users;");

    test_query("SELECT with table.column",
        "SELECT u.id, u.name FROM users AS u;");

    test_query("SELECT DISTINCT",
        "SELECT DISTINCT name FROM users;");

    // WHERE clause
    test_query("SELECT with WHERE",
        "SELECT * FROM users WHERE age > 18;");

    test_query("SELECT with complex WHERE",
        "SELECT * FROM users WHERE age >= 18 AND status = 'active';");

    test_query("SELECT with BETWEEN",
        "SELECT * FROM orders WHERE price BETWEEN 100 AND 500;");

    test_query("SELECT with IN",
        "SELECT * FROM users WHERE status IN ('active', 'pending');");

    test_query("SELECT with LIKE",
        "SELECT * FROM users WHERE name LIKE 'John%';");

    test_query("SELECT with IS NULL",
        "SELECT * FROM users WHERE deleted_at IS NULL;");

    // JOINs
    test_query("INNER JOIN",
        "SELECT u.name, o.total FROM users u INNER JOIN orders o ON u.id = o.user_id;");

    test_query("LEFT JOIN",
        "SELECT u.name, o.total FROM users u LEFT JOIN orders o ON u.id = o.user_id;");

    test_query("Multiple JOINs",
        "SELECT u.name, o.total, p.name FROM users u "
        "INNER JOIN orders o ON u.id = o.user_id "
        "INNER JOIN products p ON o.product_id = p.id;");

    test_query("Cross Join (comma)",
        "SELECT * FROM users, orders;");

    // GROUP BY and HAVING
    test_query("GROUP BY",
        "SELECT department, COUNT(*) FROM employees GROUP BY department;");

    test_query("GROUP BY with HAVING",
        "SELECT department, AVG(salary) FROM employees "
        "GROUP BY department HAVING AVG(salary) > 50000;");

    // ORDER BY
    test_query("ORDER BY ASC",
        "SELECT * FROM users ORDER BY name ASC;");

    test_query("ORDER BY DESC",
        "SELECT * FROM users ORDER BY created_at DESC;");

    test_query("ORDER BY multiple columns",
        "SELECT * FROM users ORDER BY department ASC, salary DESC;");

    // LIMIT and OFFSET
    test_query("LIMIT",
        "SELECT * FROM users LIMIT 10;");

    test_query("LIMIT with OFFSET",
        "SELECT * FROM users LIMIT 10 OFFSET 20;");

    // Aggregate functions
    test_query("Aggregate functions",
        "SELECT COUNT(*), SUM(price), AVG(price), MIN(price), MAX(price) FROM orders;");

    // Subquery in FROM
    test_query("Subquery in FROM",
        "SELECT * FROM (SELECT id, name FROM users WHERE active = TRUE) AS active_users;");

    // Complex query
    test_query("Complex query",
        "SELECT DISTINCT u.name, COUNT(o.id) AS order_count, SUM(o.total) AS total_spent "
        "FROM users u "
        "LEFT JOIN orders o ON u.id = o.user_id "
        "WHERE u.status = 'active' AND u.created_at > '2023-01-01' "
        "GROUP BY u.id, u.name "
        "HAVING COUNT(o.id) > 5 "
        "ORDER BY total_spent DESC "
        "LIMIT 100;");

    // INSERT
    test_query("INSERT single row",
        "INSERT INTO users (name, email) VALUES ('John', 'john@example.com');");

    test_query("INSERT multiple rows",
        "INSERT INTO users (name, email) VALUES ('John', 'john@example.com'), ('Jane', 'jane@example.com');");

    // UPDATE
    test_query("UPDATE",
        "UPDATE users SET status = 'inactive' WHERE last_login < '2023-01-01';");

    test_query("UPDATE multiple columns",
        "UPDATE users SET name = 'John Doe', email = 'john.doe@example.com' WHERE id = 1;");

    // DELETE
    test_query("DELETE",
        "DELETE FROM users WHERE status = 'deleted';");

    // CREATE TABLE
    test_query("CREATE TABLE",
        "CREATE TABLE products ("
        "id INT PRIMARY KEY, "
        "name VARCHAR(255) NOT NULL, "
        "price DECIMAL(10,2) DEFAULT 0, "
        "created_at TIMESTAMP"
        ");");

    // DROP TABLE
    test_query("DROP TABLE",
        "DROP TABLE temp_data;");

    // CASE expression
    test_query("CASE expression",
        "SELECT name, "
        "CASE WHEN age < 18 THEN 'minor' WHEN age < 65 THEN 'adult' ELSE 'senior' END AS age_group "
        "FROM users;");

    std::cout << "\n=== Test Suite Complete ===\n";

    return 0;
}