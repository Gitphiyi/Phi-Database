#include "sql-compiler/Parser.hpp"

namespace DB {
    std::vector<SqlNode> create_SQL_AST(std::vector<Token> tokens) //do move operations to make it apart of vector
    {
        std::vector<SqlNode> sql_statements;
        int curr_node_idx = -1;
        u32 curr_start = 0;
        u32 idx = 0;
        for (Token t : tokens) {
            if()
            query();
        }
        return sql_statements;
    }

    void query() {

    }
}