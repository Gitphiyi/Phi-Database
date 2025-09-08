#include "general/Types.hpp"
#include "sql-compiler/Lexer.hpp"
#include <iostream>

using namespace DB;

int main() {
    std::cout << "Running Lexer...\n";
    string SQL = R"(SELECT DISTINCT u.id, u.name
FROM Users AS u
INNER JOIN Orders AS o ON u.id = o.user_id
WHERE u.age >= 30 AND o.price BETWEEN 100 AND 500
ORDER BY u.name ASC;
)";
    //std::cout << SQL << std::endl;
    auto ret = tokenize_query(SQL);
    for(Token t : ret) {
        std::cout << "[" << t.type << " | " << t.value << "]\n";
    }
    return 0;
}