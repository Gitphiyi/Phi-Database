#include "sql-compiler/Parser.hpp"
#include "general/Types.hpp"

#include <iostream>
#include <cstdio>

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
            parent->children.emplace_back(SqlNode("Select Modifier"));
            parent->children[0].qualifier = tokens[i].value;
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
        if(int as_idx = check_for_alias(tokens, st, end); as_idx != -1) alias_query(parent, tokens, aliases, st, end, as_idx);
        std::cout << "invalid selection expression. Token must be . or AS.\n";
    }

    void from_query(SqlNode* parent, std::vector<Token>& tokens, std::vector<string>& aliases, int st, int end) {
        SqlNode* from_node;
        for(int i = st; i < end; ++i) {
            //reached a join
            if(tokens[i].value == "JOIN" || tokens[i].value == ",") {
                if (i == st) {
                    std::cout << "JOIN must have a left column!";
                    return;
                }
                SqlNode join_node = SqlNode();
                SqlNode left_node = from_node();
                SqlNode right_node = SqlNode();
                SqlNode on_cond_node = SqlNode(); // if empty then it is cross product
                int end_left_node = i-1;

                // udate join node
                if (tokens[i].value == ",") {
                    join_node.qualifier = "CROSS";
                }
                else if(tokens[i-1].type == KEYWORD && join_types.contains(tokens[i-1].value)) {
                    join_node.qualifier = tokens[i-1].value;
                    end_left_node--; // Make end of left node not include the Join specifier
                } 
                else {
                    join_node.qualifier = "INNER"; 
                }
                join_node.children.emplace_back(left_node);
                join_node.children.emplace_back(right_node);
                join_node.children.emplace_back(on_cond_node);

                node = join_node;

                std::printf("join type = %s JOIN \n", join_node.qualifier.c_str());
                // update left child node

                // create right child
                // create ON condition node
                // create join child node and update it as the FROM query head node
                //node->children.emplace_back(SqlNode("Join Expr"));
            }
        }
        // must be no join clauses in FROM statement, so the entire thing must be a table
        if(from_node->type == "") {

        }
        parent->children.emplace_back(from_node);
    }
    int table_expression_query(SqlNode* node, std::vector<Token>& tokens, std::vector<string>& aliases, int st) {
        // Check if alias
        // Check if single table
        if(end - st + 1 == 1) {
            node->type = "Table Reference";
            node->qualifier = tokens[st].value;
        }
        //Check for alias
        else if(int as_idx = check_for_alias(tokens, st, end); as_idx != -1) {
            alias_query(&node, tokens, aliases, st, end, as_idx);
        } 
        // //Check for Subquery
        else if(tokens[st].value == "(") {

        }
        // // Check for Condition
        // if (tokens[st].value == "ON") {

        // }
        else {
            std::cout << "Invalid table expression\n";
        }
    }

    int check_for_alias(std::vector<Token>& tokens, int st, int end) {
        int as_idx = -1;
        for(int i = st; i < end; ++i) {
            if (tokens[i].value == "AS") {
                as_idx = i;
                break;
            }
        }
        return as_idx;
    }
    void alias_query(SqlNode* parent, std::vector<Token>& tokens, std::vector<string>& aliases, int st, int end, int as_idx) {  
        if(as_idx != -1) {
            // alias the qualifier
            parent->alias = tokens[as_idx+1].value;
            aliases.push_back(tokens[end].value);
            std::cout << "left of alias size = " << (as_idx - st + 1) << std::endl;
            std::cout << tokens[st].value << std::endl;
            //if qualifier is a table reference
            if(as_idx - st == 1) {
                parent->qualifier = tokens[as_idx-1].value;
            }
            //if qualifier is a subquery
            else if(tokens[as_idx-1].value == ")") {

            }
        }
        else {
            std::cout << "expression is not aliasable\n";
        }
    }
}