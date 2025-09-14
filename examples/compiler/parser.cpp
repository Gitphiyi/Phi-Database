#include "sql-compiler/Lexer.hpp"
#include "sql-compiler/Parser.hpp"
#include <iostream>
using namespace DB;

int main() {
    string SQL = R"(SELECT DISTINCT u.id, u.name
    FROM Users AS u
    INNER JOIN Orders AS o ON u.id = o.user_id
    WHERE u.age >= 30 AND o.price BETWEEN 100 AND 500
    ORDER BY u.name ASC;
    )";
    auto ret = tokenize_query(SQL);
    create_SQL_AST(ret);
    return 0;
}