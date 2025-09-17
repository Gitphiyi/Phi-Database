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
            //std::cout << tokens[i].value << " ";
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
        sql_tree_print(root);
    }
    void select_query(SqlNode* parent, std::vector<Token>& tokens, std::vector<string>& aliases, int st, int end) {
        int i = st;
        // Should Check for SQL hints
        // Check if duplicates should be added or not
        if(tokens[i].value == "DISTINCT" || tokens[i].value == "ALL") {
            parent->children.emplace_back(SqlNode("DISTINCT/ALL"));
            ++i;
        }
        // Should just be columns now
        parent->children.emplace_back(SqlNode("Columns"));
        int parent_idx = parent->children.size() - 1; //columns node
        for(; i < end; i++) {
            int expression_st = i;
            int expression_end;
            parent->children[parent_idx].children.emplace_back(SqlNode("Column"));
            while(i < end) {
                if(tokens[i].value == ",") break;
                ++i;
            }
            expression_end = i - 1;
            SqlNode* expression_parent = &parent->children[parent_idx].children[parent->children[parent_idx].children.size()-1];
            select_expression_query(expression_parent, tokens, aliases, expression_st, expression_end);
        }
    }
    void select_expression_query(SqlNode* parent, std::vector<Token>& tokens, std::vector<string>& aliases, int st, int end) {
        // all expressions should be 3 tokens large at most
        const int SELECT_EXPR_SZ = 3;
        // is * operator or a single column
        if(end - st + 1 == 1) {
            std::cout<<"column is = " << tokens[st].value;
            if (tokens[st].value == "*") parent->qualifier = "*";
            else if(tokens[st].type == IDENTIFIER) parent->qualifier = tokens[st].value;
            else std::cout << "Select expression cannot be " << tokens[st].value << ". It must be * if it is one character\n";
            return;
        }
        // is a column with family name
        if(end - st + 1 == SELECT_EXPR_SZ && tokens[st + 1].value == ".") {
            parent->children.emplace_back(SqlNode("Table"));
            parent->children.emplace_back(SqlNode("Family Name"));
            parent->children[0].qualifier = tokens[st].value;
            parent->children[1].qualifier = tokens[end].value;
            return;
        }
        //checks for alias
        if(check_and_alias_query(parent, tokens, aliases, st, end)) return;
        std::cout << check_and_alias_query(parent, tokens, aliases, st, end);
        std::cout << "invalid selection expression. Token must be . or AS.\n";
    }

    void from_query(SqlNode* parent, std::vector<Token>& tokens, std::vector<string>& aliases, int st, int end) {
        std::vector<int> join_positions{};
        int join_pos = -1;
        for(int i = st; i < end; i++) {
            if(tokens[i].value == "," || tokens[i].value == "JOIN") {
                join_pos = i;
                break;
            }
        }
        // there should be only one column or just cross joins via commas
        if(join_pos == -1) {
            //Check for single table
            //Check for Alias
            //Check for Subquery
            // Check for Condition
        }
        // There are joins
        else {
            if(join_pos == st) {
                std::cout << "JOIN must have a left column!";
                return;
            }
            // create join child node and recurse through left and right side to obtain the left and right expressions that need to be joined
            parent->children.emplace_back(SqlNode("Join Expr"));
            if (tokens[join_pos].value == ",") {
                parent->children[parent->children.size()-1].join_type = "CROSS";
                from_query(&parent->children[parent->children.size()-1], tokens, aliases, st, join_pos);  
            }
            else if(tokens[join_pos-1].type == KEYWORD && join_types.contains(tokens[join_pos-1].value)) {
                parent->children[parent->children.size()-1].join_type = tokens[join_pos-1].value;
                from_query(&parent->children[parent->children.size()-1], tokens, aliases, st, join_pos-1);
            } 
            else {
                parent->children[parent->children.size()-1].join_type = "INNER"; 
                from_query(&parent->children[parent->children.size()-1], tokens, aliases, st, join_pos);  
            }
            from_query(&parent->children[parent->children.size()-1], tokens, aliases, join_pos + 1, end);  
        }
    }
    bool check_and_alias_query(SqlNode* parent, std::vector<Token>& tokens, std::vector<string>& aliases, int st, int end) {
        int as_idx = -1;
        for(int i = st; i < end; ++i) {
            if (tokens[i].value == "AS") {
                as_idx = i;
                break;
            }
        }

        //is alias
        if(as_idx != -1) {
            parent->alias = tokens[end].value;
            aliases.push_back(tokens[end].value);
            return true;
        }
        return false;
    }

}