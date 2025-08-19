#pragma once

#include "general/Types.hpp"

#include <unordered_set>

namespace DB {
    const std::unordered_set<string> keywords = {
        "SELECT", "FROM", "WHERE", "INSERT", "UPDATE",
        "DELETE", "CREATE", "TABLE", "VALUES", "INTO"
    };

    enum TokenType {
        KEYWORD,
        IDENTIFIER,
        NUMBER,
        STRING,
        SYMBOL,     // (, ), ,, ;, etc
        OPERATOR,   // =, >, <, >=, <=
        END
    };

    struct Token {
        TokenType   type;
        string      value;
    };

    struct Query {
        string query;
        
    };

    class QueryProcessor {
        public:

        private:
            string  query;
    };
}