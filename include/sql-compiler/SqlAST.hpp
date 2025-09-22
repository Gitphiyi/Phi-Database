#pragma once

#include "general/Types.hpp"

#include <iostream>
#include <vector>
#include <deque>
#include <string>

enum NodeType {
    DEFAULT,
    SELECT_STMT,
    SELECT_CLAUSE,
    FROM_CLAUSE,
    WHERE_CLAUSE,
};

struct SqlNode {
    string                      type;
    string                      qualifier;
    std::vector<string>         clauses{}; //depends on statement. Suppose this can be null
    std::vector<SqlNode>        children{};
    string                      alias;

    //FROM clause specific fields
    SqlNode() : type(""), qualifier(""), alias("") {}
    SqlNode(string t) : type(t), qualifier(""), alias("") {}
};

inline void tree_print_helper(SqlNode* root, const string& prefix = "", bool is_last = true, bool is_root = true) {
    string child_prefix;
    if(!is_root) {
        std::cout << prefix << (is_last ? " └── " : " ├── ");
        child_prefix = prefix;
        child_prefix += (is_last ? "     " : " │   ");
    }
    
    std::cout << root->type;
    if(root->alias != "") std::cout << " { alias: " << root->alias << " }";
    if(root->qualifier != "") std::cout << " { qualifer: " << root->qualifier << " }";
    std::cout << std::endl;

    for (size_t i = 0; i < root->children.size(); i++) {
        bool last_child = (i == root->children.size() - 1);
        tree_print_helper(&root->children[i], child_prefix, last_child, false);
    }
}
static void sql_tree_print(SqlNode* root) {
    std::cout << "\nPrinting SQL AST: \n";
    tree_print_helper(root, "", false, true);
}

static void level_sql_tree_print(SqlNode* root) {
    std::cout << "Printing SQL AST in level order: \n";
    std::deque<SqlNode*> heap{};
    heap.push_back(root);
    int level = 0;
    while(!heap.empty()) {
        int level_size = heap.size();
        for (int i = 0; i < level_size; i++) {
            SqlNode* temp = heap.front();
            heap.pop_front();

            std::string dashes(level*2, '-');
            std::cout << "|" << dashes << temp->type << "\n";

            for (SqlNode& child : temp->children) {
                heap.push_back(&child);
            }
        }
        ++level;
    }
}

struct RANode {
    NodeType                    type;
    std::vector<string>         clauses{}; //depends on statement. Suppose this can be null
    std::vector<SqlNode>        children{};
    //FROM clause fields
    string                      join_type;

    RANode(NodeType t) : type(t), join_type("") {}
};
