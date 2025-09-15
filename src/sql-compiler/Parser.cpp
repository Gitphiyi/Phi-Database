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
            sql_statements.emplace_back(SqlNode(tokens[0].value));
            start_tokens[tokens[0].value](&sql_statements[0], tokens, 0); // run the statement dependent parser
        }
        return sql_statements;
    }

    void select_query(SqlNode* root, std::vector<Token>& tokens, int st) {
        //std::cout << "SELECT grammar query checker\n";
        bool stop = false;
        int i = 1;
        std::cout << tokens[0].value << " size: " << tokens.size() << "\n";

        while ( i < tokens.size() && tokens[i].value != ";") {
            std::cout << tokens[i].value << ", ";
            if(tokens[i].value == "SELECT") {
                root->children.emplace_back(SqlNode("Select Clause"));
            }
            else if(tokens[i].value == "FROM") {
                root->children.emplace_back(SqlNode("From Clause"));
            } 
            else if(tokens[i].value == "WHERE") {
                root->children.emplace_back(SqlNode("Where Clause"));
            }
            // do one for GROUP BY
            ++i;
        }
        std::cout << std::endl;
        level_sql_tree_print(root);
    }
    void select_expression_query(SqlNode* parent, std::vector<Token>& tokens, int st) {
                int i = 1;
            // Check if duplicates should be added or not
            if(tokens[i].value == "DISTINCT" || tokens[i].value == "ALL") {

            }
            // Check if * is used
            else if(tokens[i].value == "*") {

            }
            // start of new expression
            else if(tokens[i].value == ",") {

            }
            // should be a selection expression
            else {
                
            }
    }
}