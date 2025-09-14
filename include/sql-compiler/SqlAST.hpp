#pragma once

#include "general/Types.hpp"
#include "vector"

enum NodeType {
    DEFAULT,
    SELECT_CLAUSE,
    FROM_CLAUSE,
    WHERE_CLAUSE,
};

struct SqlNode {
    NodeType                    type;
    std::vector<string>         clauses{}; //depends on statement. Suppose this can be null

    // u32                         start{0};
    // u32                         end{0};
    std::vector<SqlNode*>       children{};
    SqlNode(NodeType t) : type(t) {}
};

struct RANode {
    NodeType                    type;
    std::vector<string>         clauses{}; //depends on statement. Suppose this can be null
    // u32                         start{0};
    // u32                         end{0};
    std::vector<SqlNode*>       children{};
    RANode(NodeType t) : type(t) {}
};