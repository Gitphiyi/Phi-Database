#include "sql-compiler/Parser.hpp"

#include <iostream>

namespace DB {
    std::vector<SqlNode> create_SQL_AST(std::vector<Token> tokens) //do move operations to make it apart of vector
    {
        std::vector<SqlNode> sql_statements;
        int curr_node_idx = -1;
        u32 curr_start = 0;
        u32 idx = 0;
        if(start_tokens.contains(tokens[0].value)) {
            sql_statements.emplace_back(SqlNode(NodeType::SELECT_CLAUSE));
            start_tokens[tokens[0].value](&sql_statements[0], tokens);
        }
        return sql_statements;
    }

    void select_query(SqlNode* root, std::vector<Token> tokens) {
        std::cout << "SELECT grammar query checker\n";
    }
}