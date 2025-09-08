#include "sql-compiler/Parser.hpp"

namespace DB {
    std::vector<SqlNode> create_SQL_AST(std::vector<Token> tokens) //do move operations to make it apart of vector
    {
        std::vector<SqlNode> sql_statements;
        int curr_node_idx = -1;
        u32 curr_start = 0;
        u32 idx = 0;
        for (Token t : tokens) {
            if(t.type == KEYWORD) {
                if(t.value == "SELECT") {
                    if(sql_statements.size() > 0) {
                        sql_statements[sql_statements.size() - 1].end = idx;
                    }
                    sql_statements.emplace_back(SqlNode(SELECT_CLAUSE));
                    curr_start = idx+1;
                } 
                else if(t.value == "FROM") {
                    if(sql_statements.size() > 0) {
                        sql_statements[sql_statements.size() - 1].end = idx;
                    }
                    sql_statements.emplace_back(SqlNode(FROM_CLAUSE));
                }
                else if(t.value == "WHERE") {
                    if(sql_statements.size() > 0) {
                        sql_statements[sql_statements.size() - 1].end = idx;
                    }
                    sql_statements.emplace_back(SqlNode(WHERE_CLAUSE));
                }
            }
            idx += t.value.size();
        }
        return sql_statements;
    }
}