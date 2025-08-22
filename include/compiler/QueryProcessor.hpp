#pragma once

#include "general/Types.hpp"

#include <unordered_set>

namespace DB {
    const std::unordered_set<string> keywords = {
        "SELECT", "FROM", "WHERE", "INSERT", "UPDATE",
        "DELETE", "CREATE", "TABLE", "VALUES", "INTO",
        "DISTINCT", "ORDER", "GROUP", "BY", "HAVING", "OFFSET",
        "INNER", "LEFT", "OUTER", "RIGHT", "FULL", "JOIN",
        "ON", "USING", "SET", "ALTER", "DROP", "TRUNCATE",
        "PRIMARY", "FOREIGN", "KEY", "UNIQUE", "CHECK", "DEFAULT",
        "NOT", "NULL", "BEGIN", "TRANSACTION", "COMMIT", "ROLLBACK",
        "SAVEPOINT", "AS", "IN", "IS", "NULL", "NOT", "BETWEEN",
        "EXISTS", "ANY", "ALL", "WITH", "EXCEPT", "UNION", "ALL",
        "CAST", "CASE", "WHEN", "THEN", "ELSE", "END"
    };

    const std::unordered_set<string> comparison_ops = {
        "=", ">", "<", ">=", "<="
    };

    const std::unordered_set<string> symbols = {
        "(", ")", ",", ";", "", "*"
    };

    enum TokenType {
        KEYWORD,
        IDENTIFIER,
        NUMBER,
        STRING,
        SYMBOL,   
        OPERATOR,
    };

    struct Token {
        TokenType   type;
        string      value;
    };

    struct Query {
        string query;
    };

    std::vector<Token> tokenize_query(string& query);
}