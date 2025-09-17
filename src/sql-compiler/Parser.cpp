#include "sql-compiler/Parser.hpp"
#include "general/Types.hpp"

#include <iostream>

namespace DB {
    std::vector<SqlNode> create_SQL_AST(std::vector<Token> tokens) //do move operations to make it apart of vector
    {
        std::vector<SqlNode> sql_statements;
        int curr_node_idx = -1;
        u32 curr_start = 0;
        u32 idx = 0;
        if(start_tokens.contains(tokens[0].value)) {
            std::vector<string> stmt_aliases;
            sql_statements.emplace_back(SqlNode(tokens[0].value));
            start_tokens[tokens[0].value](&sql_statements[0], tokens, stmt_aliases, 0); // run the statement dependent parser
        }
        return sql_statements;
    }

    void sql_query(SqlNode* root, std::vector<Token>& tokens, std::vector<string>& aliases, int st) {
        //std::cout << "SELECT grammar query checker\n";
        int i = 0;
        std::cout << tokens[0].value << " size: " << tokens.size() << "\n";

        while ( i < tokens.size() && tokens[i].value != ";") {
            std::cout << tokens[i].value << " ";
            // For these conditional statements, determine the start and end tokens of the clause and recursively create
            // nodes until it is over
            if(tokens[i].value == "SELECT") {
                root->children.emplace_back(SqlNode("Select Clause"));
                int parent_idx = root->children.size() - 1;
                SqlNode* parent = &root->children[parent_idx];
                int st = i + 1;
                int end;
                while(tokens[i].value != "FROM") {
                    if(i >= tokens.size() || tokens[i].value == ";") {
                        std::cout << "Cannot have SELECT statement without FROM clause";
                        return;
                    }
                    ++i;
                }
                end = i;
                select_query(parent, tokens, aliases, st, end);
            }
            if(tokens[i].value == "FROM") {
                root->children.emplace_back(SqlNode("From Clause"));
                int parent_idx = root->children.size() - 1;
                SqlNode* parent = &root->children[parent_idx];
                int st = i + 1;
                int end;
                while(tokens[i].value != "WHERE" && tokens[i].value != ";") {
                    if(i >= tokens.size()) {
                        std::cout << "The FROM clause doesn't end with ; or WHERE";
                        return;
                    }
                    ++i;
                }
                end = i;
                from_query(parent, tokens, aliases, st, end);
            } 
            if(tokens[i].value == "WHERE") {
                root->children.emplace_back(SqlNode("Where Clause"));
            }
            // do one for GROUP BY
            ++i;
        }
        std::cout << std::endl;
        level_sql_tree_print(root);
        sql_tree_print(root);
    }
    void select_query(SqlNode* parent, std::vector<Token>& tokens, std::vector<string>& aliases, int st, int end) {
        int i = st;
        for(int i = st; i < end; i++) {
            // Check if duplicates should be added or not
            if(tokens[i].value == "DISTINCT" || tokens[i].value == "ALL") {
                parent->children.emplace_back(SqlNode("duplication operator"));
                // do this or make it a field of the node
            }
            // should be a selection expression
            else {
                parent->children.emplace_back(SqlNode("Column"));
                int parent_idx = parent->children.size() - 1;
                SqlNode* expression_parent = &parent->children[parent_idx];
                int expression_st = i;
                int expression_end;
                while(i < end) {
                    if(tokens[i].value == ",") break;
                    //std::cout << "make column node\n";
                    ++i;
                }
                expression_end = i - 1;
                select_expression_query(expression_parent, tokens, aliases, expression_st, expression_end);
            }
        }
    }
    void select_expression_query(SqlNode* parent, std::vector<Token>& tokens, std::vector<string>& aliases, int st, int end) {
        // all expressions should be 3 tokens large at most
        const int SELECT_EXPR_SZ = 3;
        // is * operator
        if(end - st + 1 == 1) {
            if (tokens[st].value == "*") parent->children.emplace_back(SqlNode("All rows"));
            else std::cout << "Select expression cannot be " << tokens[st].value << ". It must be * if it is one character";
            return;
        }
        // is a column
        if(end - st + 1 == SELECT_EXPR_SZ && tokens[st + 1].value == ".") {
            parent->children.emplace_back(SqlNode("col from family name"));
            return;
        }
        //uses an alias
        int as_idx = -1;
        for(int i = st; i < end; ++i) {
            if (tokens[i].value == "AS") {
                as_idx = i;
                break;
            }
        }
        if(as_idx != -1) {
            parent->children.emplace_back(SqlNode("col and set alias"));
            aliases.push_back(tokens[as_idx + 1].value);
            return;
        }
        std::cout << "invalid selection expression. Token must be . or AS";
    }

    void from_query(SqlNode* parent, std::vector<Token>& tokens, std::vector<string>& aliases, int st, int end) {
        std::unordered_set<int> join_positions{};
        for(int i = st; i < end; i++) {
            if(tokens[i].type == KEYWORD && tokens[i].value == "JOIN") {
                join_positions.insert(i);
            }
        }

        if(join_positions.size() == 0) {
        }
        // there should be only one column or just cross joins via commas
        else {
            for(int join_pos : join_positions) {
                std::cout << "JOIN exists at idx: " << join_pos << ", ";
            }
            std::cout << std::endl;
        }
    }
}