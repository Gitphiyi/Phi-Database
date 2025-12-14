#include "query-executor/QueryExecutor.hpp"
#include "page-manager/DbFile.hpp"

#include <iostream>

using namespace DB;

int main() {
    std::cout << "=== Phi-Database Query Executor Demo ===" << std::endl;
    std::cout << std::endl;

    DbFile::initialize(true);
    Catalog catalog;
    QueryExecutor executor(catalog);

    std::cout << "1. Creating a table..." << std::endl;
    string create_sql = "CREATE TABLE users (id INT PRIMARY KEY, name VARCHAR, age INT)";
    std::cout << "   SQL: " << create_sql << std::endl;

    QueryResult result = executor.execute(create_sql);
    if (result.success) {
        std::cout << "   Table 'users' created successfully!" << std::endl;
    } else {
        std::cout << "   Error: " << result.error_message << std::endl;
    }
    std::cout << std::endl;

    std::cout << "2. Parsing a SELECT query..." << std::endl;
    string select_sql = "SELECT id, name FROM users WHERE age > 18";
    std::cout << "   SQL: " << select_sql << std::endl;

    RANodePtr ra_tree = compile_sql(select_sql);
    if (ra_tree) {
        ra_tree_print(ra_tree);
    }
    std::cout << std::endl;

    std::cout << "3. Parsing a JOIN query..." << std::endl;
    string join_sql = "SELECT u.name, o.total FROM users u INNER JOIN orders o ON u.id = o.user_id";
    std::cout << "   SQL: " << join_sql << std::endl;

    ra_tree = compile_sql(join_sql);
    if (ra_tree) {
        ra_tree_print(ra_tree);
    }
    std::cout << std::endl;

    std::cout << "4. Parsing an INSERT statement..." << std::endl;
    string insert_sql = "INSERT INTO users (id, name, age) VALUES (1, 'Alice', 25), (2, 'Bob', 30)";
    std::cout << "   SQL: " << insert_sql << std::endl;

    ra_tree = compile_sql(insert_sql);
    if (ra_tree) {
        std::cout << "   Operation: " << ra_type_to_string(ra_tree->type) << std::endl;
        std::cout << "   Table: " << ra_tree->table_name << std::endl;
        std::cout << "   Rows to insert: " << ra_tree->insert_values.size() << std::endl;
    }
    std::cout << std::endl;

    std::cout << "5. Parsing a complex SELECT query..." << std::endl;
    string complex_sql = "SELECT department, COUNT(*) as emp_count "
                         "FROM employees "
                         "WHERE salary > 50000 "
                         "GROUP BY department "
                         "HAVING COUNT(*) > 5 "
                         "ORDER BY emp_count DESC "
                         "LIMIT 10";
    std::cout << "   SQL: " << complex_sql << std::endl;

    ra_tree = compile_sql(complex_sql);
    if (ra_tree) {
        ra_tree_print(ra_tree);
    }
    std::cout << std::endl;

    std::cout << "=== Demo Complete ===" << std::endl;

    return 0;
}
