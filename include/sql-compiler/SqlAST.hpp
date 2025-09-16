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
    string                    type;
    std::vector<string>         clauses{}; //depends on statement. Suppose this can be null

    // u32                         start{0};
    // u32                         end{0};
    std::vector<SqlNode>       children{};
    SqlNode(string t) : type(t) {}
};

inline void tree_print_helper(SqlNode* root, int level) {
    std::string dashes(level, '-');
    std::cout << dashes << "|" << root->type << "\n";
    for(SqlNode& child : root->children) {
        tree_print_helper(&child, level + 1);
    }
}
static void sql_tree_print(SqlNode* root) {
    std::cout << "Printing SQL AST: \n";
    tree_print_helper(root, 0);
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

            std::string dashes(level, '-');
            std::cout << dashes << "|" << temp->type << "\n";

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
    // u32                         start{0};
    // u32                         end{0};
    std::vector<SqlNode>       children{};
    RANode(NodeType t) : type(t) {}
};
