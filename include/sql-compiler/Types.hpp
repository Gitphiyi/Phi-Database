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
        "SAVEPOINT", "AS", "IN", "IS", "BETWEEN",
        "EXISTS", "ANY", "ALL", "WITH", "EXCEPT", "UNION",
        "CAST", "CASE", "WHEN", "THEN", "ELSE", "END",
        "ASC", "DESC", "LIMIT", "CROSS", "NATURAL", "LIKE",
        "TRUE", "FALSE", "AND", "OR", "NULLS"
    };

    const std::unordered_set<string> sql_ops = {
        "<>", "=", ">", "<", ">=", "<=", "/", "%", "+", "-", "::", "||"
    };

    const std::unordered_set<string> symbols = {
        "(", ")", ",", ".", ";", "", "*", "[", "]"
    };

    const std::unordered_set<string> join_types = {
        "INNER", "OUTER", "LEFT", "RIGHT", "NATURAL", "FULL"
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
}