#pragma once

#include "general/Types.hpp"

#include <iostream>
#include <vector>
#include <deque>
#include <string>
#include <memory>
#include <variant>

// ============================================================================
// SQL AST Node Types (for parsing)
// ============================================================================

enum class SqlNodeType {
    // Statements
    SELECT_STMT,
    INSERT_STMT,
    UPDATE_STMT,
    DELETE_STMT,
    CREATE_TABLE_STMT,
    DROP_TABLE_STMT,

    // Clauses
    SELECT_CLAUSE,
    FROM_CLAUSE,
    WHERE_CLAUSE,
    GROUP_BY_CLAUSE,
    HAVING_CLAUSE,
    ORDER_BY_CLAUSE,
    LIMIT_CLAUSE,

    // Expressions
    COLUMN_REF,
    TABLE_REF,
    LITERAL,
    BINARY_EXPR,
    UNARY_EXPR,
    FUNCTION_CALL,
    SUBQUERY,
    CASE_EXPR,

    // Join
    JOIN_EXPR,

    // Misc
    ALIAS,
    COLUMN_LIST,
    TABLE_LIST,
    ORDER_ITEM,
    COLUMN_DEF,

    UNKNOWN
};

// Forward declarations
struct SqlNode;
struct Expression;

// ============================================================================
// Expression types for WHERE, ON, HAVING clauses
// ============================================================================

enum class ExprType {
    LITERAL_INT,
    LITERAL_FLOAT,
    LITERAL_STRING,
    LITERAL_BOOL,
    LITERAL_NULL,
    COLUMN_REF,
    BINARY_OP,
    UNARY_OP,
    FUNCTION_CALL,
    SUBQUERY,
    IN_LIST,
    BETWEEN,
    CASE_WHEN,
    EXISTS,
    IS_NULL
};

enum class BinaryOp {
    // Comparison
    EQ,        // =
    NE,        // <> or !=
    LT,        // <
    LE,        // <=
    GT,        // >
    GE,        // >=

    // Logical
    AND,
    OR,

    // Arithmetic
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,

    // String
    CONCAT,    // ||
    LIKE
};

enum class UnaryOp {
    NOT,
    MINUS,
    PLUS,
    IS_NULL,
    IS_NOT_NULL
};

struct Expression {
    ExprType type;

    // For literals
    std::variant<int64_t, double, string, bool, std::monostate> literal_value;

    // For column reference: table.column or just column
    string table_name;
    string column_name;

    // For binary/unary operations
    BinaryOp binary_op;
    UnaryOp unary_op;
    std::vector<std::shared_ptr<Expression>> children;

    // For function calls
    string function_name;

    // For IN list
    std::vector<std::shared_ptr<Expression>> in_list;

    // Alias
    string alias;

    Expression() : type(ExprType::LITERAL_NULL), binary_op(BinaryOp::EQ), unary_op(UnaryOp::NOT) {}

    static std::shared_ptr<Expression> makeColumnRef(const string& col, const string& tbl = "") {
        auto e = std::make_shared<Expression>();
        e->type = ExprType::COLUMN_REF;
        e->column_name = col;
        e->table_name = tbl;
        return e;
    }

    static std::shared_ptr<Expression> makeLiteralInt(int64_t val) {
        auto e = std::make_shared<Expression>();
        e->type = ExprType::LITERAL_INT;
        e->literal_value = val;
        return e;
    }

    static std::shared_ptr<Expression> makeLiteralFloat(double val) {
        auto e = std::make_shared<Expression>();
        e->type = ExprType::LITERAL_FLOAT;
        e->literal_value = val;
        return e;
    }

    static std::shared_ptr<Expression> makeLiteralString(const string& val) {
        auto e = std::make_shared<Expression>();
        e->type = ExprType::LITERAL_STRING;
        e->literal_value = val;
        return e;
    }

    static std::shared_ptr<Expression> makeLiteralBool(bool val) {
        auto e = std::make_shared<Expression>();
        e->type = ExprType::LITERAL_BOOL;
        e->literal_value = val;
        return e;
    }

    static std::shared_ptr<Expression> makeLiteralNull() {
        auto e = std::make_shared<Expression>();
        e->type = ExprType::LITERAL_NULL;
        e->literal_value = std::monostate{};
        return e;
    }

    static std::shared_ptr<Expression> makeBinaryOp(BinaryOp op,
            std::shared_ptr<Expression> left, std::shared_ptr<Expression> right) {
        auto e = std::make_shared<Expression>();
        e->type = ExprType::BINARY_OP;
        e->binary_op = op;
        e->children.push_back(left);
        e->children.push_back(right);
        return e;
    }

    static std::shared_ptr<Expression> makeUnaryOp(UnaryOp op, std::shared_ptr<Expression> operand) {
        auto e = std::make_shared<Expression>();
        e->type = ExprType::UNARY_OP;
        e->unary_op = op;
        e->children.push_back(operand);
        return e;
    }

    static std::shared_ptr<Expression> makeFunctionCall(const string& name,
            std::vector<std::shared_ptr<Expression>> args) {
        auto e = std::make_shared<Expression>();
        e->type = ExprType::FUNCTION_CALL;
        e->function_name = name;
        e->children = std::move(args);
        return e;
    }
};

using ExprPtr = std::shared_ptr<Expression>;

// ============================================================================
// Relational Algebra Node Types (for execution)
// ============================================================================

enum class RANodeType {
    // Leaf operators
    TABLE_SCAN,       // Scan a base table

    // Unary operators
    PROJECT,          // π (projection)
    SELECT_OP,        // σ (selection/filter)
    RENAME,           // ρ (rename)
    DISTINCT,         // Remove duplicates
    SORT,             // Order by
    LIMIT_OP,         // Limit/offset
    GROUP_BY,         // γ (grouping with aggregation)

    // Binary operators
    CROSS_PRODUCT,    // × (cartesian product)
    INNER_JOIN,       // ⋈ (natural/inner join)
    LEFT_JOIN,        // ⟕ (left outer join)
    RIGHT_JOIN,       // ⟖ (right outer join)
    FULL_JOIN,        // ⟗ (full outer join)
    UNION_OP,         // ∪ (union)
    INTERSECT_OP,     // ∩ (intersection)
    EXCEPT_OP,        // − (set difference)

    // DML
    INSERT_OP,
    UPDATE_OP,
    DELETE_OP,

    // DDL
    CREATE_TABLE_OP,
    DROP_TABLE_OP
};

struct SortSpec {
    string column;
    string table;
    bool ascending = true;
    bool nulls_first = false;
};

struct ColumnDef {
    string name;
    string type_name;
    bool nullable = true;
    bool primary_key = false;
    ExprPtr default_value;
};

// Forward declaration
struct RANode;
using RANodePtr = std::shared_ptr<RANode>;

struct RANode {
    RANodeType type;

    // For TABLE_SCAN
    string table_name;
    string table_alias;

    // For PROJECT: list of expressions to project
    std::vector<ExprPtr> projections;
    std::vector<string> projection_aliases;
    bool select_all = false;  // SELECT *

    // For SELECT_OP (filter): the predicate
    ExprPtr predicate;

    // For JOINs: the join condition
    ExprPtr join_condition;

    // For GROUP_BY
    std::vector<ExprPtr> group_by_exprs;
    ExprPtr having_predicate;

    // For SORT
    std::vector<SortSpec> order_specs;

    // For LIMIT
    int64_t limit_count = -1;
    int64_t offset_count = 0;

    // For DISTINCT
    bool is_distinct = false;

    // For INSERT
    std::vector<string> insert_columns;
    std::vector<std::vector<ExprPtr>> insert_values;  // Multiple rows

    // For UPDATE
    std::vector<std::pair<string, ExprPtr>> update_assignments;

    // For CREATE TABLE
    std::vector<ColumnDef> column_defs;

    // Children (0 for leaf, 1 for unary, 2 for binary)
    RANodePtr left;
    RANodePtr right;

    RANode() : type(RANodeType::TABLE_SCAN), select_all(false),
               limit_count(-1), offset_count(0), is_distinct(false) {}
    RANode(RANodeType t) : type(t), select_all(false),
               limit_count(-1), offset_count(0), is_distinct(false) {}

    // Factory methods
    static RANodePtr makeTableScan(const string& table, const string& alias = "") {
        auto n = std::make_shared<RANode>(RANodeType::TABLE_SCAN);
        n->table_name = table;
        n->table_alias = alias.empty() ? table : alias;
        return n;
    }

    static RANodePtr makeProject(RANodePtr child, std::vector<ExprPtr> projs,
                                  std::vector<string> aliases = {}) {
        auto n = std::make_shared<RANode>(RANodeType::PROJECT);
        n->left = child;
        n->projections = std::move(projs);
        n->projection_aliases = std::move(aliases);
        return n;
    }

    static RANodePtr makeSelect(RANodePtr child, ExprPtr pred) {
        auto n = std::make_shared<RANode>(RANodeType::SELECT_OP);
        n->left = child;
        n->predicate = pred;
        return n;
    }

    static RANodePtr makeJoin(RANodeType join_type, RANodePtr left, RANodePtr right, ExprPtr cond = nullptr) {
        auto n = std::make_shared<RANode>(join_type);
        n->left = left;
        n->right = right;
        n->join_condition = cond;
        return n;
    }

    static RANodePtr makeGroupBy(RANodePtr child, std::vector<ExprPtr> groups, ExprPtr having = nullptr) {
        auto n = std::make_shared<RANode>(RANodeType::GROUP_BY);
        n->left = child;
        n->group_by_exprs = std::move(groups);
        n->having_predicate = having;
        return n;
    }

    static RANodePtr makeSort(RANodePtr child, std::vector<SortSpec> specs) {
        auto n = std::make_shared<RANode>(RANodeType::SORT);
        n->left = child;
        n->order_specs = std::move(specs);
        return n;
    }

    static RANodePtr makeLimit(RANodePtr child, int64_t limit, int64_t offset = 0) {
        auto n = std::make_shared<RANode>(RANodeType::LIMIT_OP);
        n->left = child;
        n->limit_count = limit;
        n->offset_count = offset;
        return n;
    }

    static RANodePtr makeDistinct(RANodePtr child) {
        auto n = std::make_shared<RANode>(RANodeType::DISTINCT);
        n->left = child;
        return n;
    }
};

// ============================================================================
// Legacy SqlNode (kept for compatibility)
// ============================================================================

struct SqlNode {
    string                      type;
    string                      qualifier;
    std::vector<string>         clauses{};
    std::vector<SqlNode>        children{};
    string                      alias;

    SqlNode() : type(""), qualifier(""), alias("") {}
    SqlNode(string t) : type(t), qualifier(""), alias("") {}
};

// ============================================================================
// Debug/Print utilities
// ============================================================================

inline void tree_print_helper(SqlNode* root, const string& prefix = "", bool is_last = true, bool is_root = true) {
    string child_prefix;
    if(!is_root) {
        std::cout << prefix << (is_last ? " └── " : " ├── ");
        child_prefix = prefix;
        child_prefix += (is_last ? "     " : " │   ");
    }

    std::cout << root->type;
    if(root->alias != "") std::cout << " { alias: " << root->alias << " }";
    if(root->qualifier != "") std::cout << " { qualifier: " << root->qualifier << " }";
    std::cout << std::endl;

    for (size_t i = 0; i < root->children.size(); i++) {
        bool last_child = (i == root->children.size() - 1);
        tree_print_helper(&root->children[i], child_prefix, last_child, false);
    }
}

inline void sql_tree_print(SqlNode* root) {
    std::cout << "\nPrinting SQL AST: \n";
    tree_print_helper(root, "", false, true);
}

inline string ra_type_to_string(RANodeType type) {
    switch(type) {
        case RANodeType::TABLE_SCAN: return "TableScan";
        case RANodeType::PROJECT: return "Project";
        case RANodeType::SELECT_OP: return "Select";
        case RANodeType::RENAME: return "Rename";
        case RANodeType::DISTINCT: return "Distinct";
        case RANodeType::SORT: return "Sort";
        case RANodeType::LIMIT_OP: return "Limit";
        case RANodeType::GROUP_BY: return "GroupBy";
        case RANodeType::CROSS_PRODUCT: return "CrossProduct";
        case RANodeType::INNER_JOIN: return "InnerJoin";
        case RANodeType::LEFT_JOIN: return "LeftJoin";
        case RANodeType::RIGHT_JOIN: return "RightJoin";
        case RANodeType::FULL_JOIN: return "FullJoin";
        case RANodeType::UNION_OP: return "Union";
        case RANodeType::INTERSECT_OP: return "Intersect";
        case RANodeType::EXCEPT_OP: return "Except";
        case RANodeType::INSERT_OP: return "Insert";
        case RANodeType::UPDATE_OP: return "Update";
        case RANodeType::DELETE_OP: return "Delete";
        case RANodeType::CREATE_TABLE_OP: return "CreateTable";
        case RANodeType::DROP_TABLE_OP: return "DropTable";
        default: return "Unknown";
    }
}

inline string expr_to_string(const ExprPtr& expr) {
    if (!expr) return "NULL";

    switch(expr->type) {
        case ExprType::LITERAL_INT:
            return std::to_string(std::get<int64_t>(expr->literal_value));
        case ExprType::LITERAL_FLOAT:
            return std::to_string(std::get<double>(expr->literal_value));
        case ExprType::LITERAL_STRING:
            return "'" + std::get<string>(expr->literal_value) + "'";
        case ExprType::LITERAL_BOOL:
            return std::get<bool>(expr->literal_value) ? "TRUE" : "FALSE";
        case ExprType::LITERAL_NULL:
            return "NULL";
        case ExprType::COLUMN_REF:
            return expr->table_name.empty() ? expr->column_name :
                   expr->table_name + "." + expr->column_name;
        case ExprType::BINARY_OP: {
            string op_str;
            switch(expr->binary_op) {
                case BinaryOp::EQ: op_str = "="; break;
                case BinaryOp::NE: op_str = "<>"; break;
                case BinaryOp::LT: op_str = "<"; break;
                case BinaryOp::LE: op_str = "<="; break;
                case BinaryOp::GT: op_str = ">"; break;
                case BinaryOp::GE: op_str = ">="; break;
                case BinaryOp::AND: op_str = "AND"; break;
                case BinaryOp::OR: op_str = "OR"; break;
                case BinaryOp::ADD: op_str = "+"; break;
                case BinaryOp::SUB: op_str = "-"; break;
                case BinaryOp::MUL: op_str = "*"; break;
                case BinaryOp::DIV: op_str = "/"; break;
                case BinaryOp::MOD: op_str = "%"; break;
                case BinaryOp::CONCAT: op_str = "||"; break;
                case BinaryOp::LIKE: op_str = "LIKE"; break;
            }
            return "(" + expr_to_string(expr->children[0]) + " " + op_str + " " +
                   expr_to_string(expr->children[1]) + ")";
        }
        case ExprType::UNARY_OP: {
            string op_str;
            switch(expr->unary_op) {
                case UnaryOp::NOT: op_str = "NOT "; break;
                case UnaryOp::MINUS: op_str = "-"; break;
                case UnaryOp::PLUS: op_str = "+"; break;
                case UnaryOp::IS_NULL: return expr_to_string(expr->children[0]) + " IS NULL";
                case UnaryOp::IS_NOT_NULL: return expr_to_string(expr->children[0]) + " IS NOT NULL";
            }
            return op_str + expr_to_string(expr->children[0]);
        }
        case ExprType::FUNCTION_CALL: {
            string args;
            for (size_t i = 0; i < expr->children.size(); i++) {
                if (i > 0) args += ", ";
                args += expr_to_string(expr->children[i]);
            }
            return expr->function_name + "(" + args + ")";
        }
        default:
            return "?";
    }
}

inline void ra_tree_print_helper(const RANodePtr& node, const string& prefix = "",
                                  bool is_last = true, bool is_root = true) {
    if (!node) return;

    string child_prefix;
    if (!is_root) {
        std::cout << prefix << (is_last ? " └── " : " ├── ");
        child_prefix = prefix + (is_last ? "     " : " │   ");
    }

    std::cout << ra_type_to_string(node->type);

    // Print node-specific info
    switch(node->type) {
        case RANodeType::TABLE_SCAN:
            std::cout << " [" << node->table_name;
            if (!node->table_alias.empty() && node->table_alias != node->table_name)
                std::cout << " AS " << node->table_alias;
            std::cout << "]";
            break;
        case RANodeType::PROJECT:
            std::cout << " [";
            if (node->select_all) {
                std::cout << "*";
            } else {
                for (size_t i = 0; i < node->projections.size(); i++) {
                    if (i > 0) std::cout << ", ";
                    std::cout << expr_to_string(node->projections[i]);
                    if (i < node->projection_aliases.size() && !node->projection_aliases[i].empty())
                        std::cout << " AS " << node->projection_aliases[i];
                }
            }
            std::cout << "]";
            break;
        case RANodeType::SELECT_OP:
            std::cout << " [" << expr_to_string(node->predicate) << "]";
            break;
        case RANodeType::INNER_JOIN:
        case RANodeType::LEFT_JOIN:
        case RANodeType::RIGHT_JOIN:
        case RANodeType::FULL_JOIN:
            if (node->join_condition)
                std::cout << " [ON " << expr_to_string(node->join_condition) << "]";
            break;
        case RANodeType::SORT:
            std::cout << " [";
            for (size_t i = 0; i < node->order_specs.size(); i++) {
                if (i > 0) std::cout << ", ";
                if (!node->order_specs[i].table.empty())
                    std::cout << node->order_specs[i].table << ".";
                std::cout << node->order_specs[i].column;
                std::cout << (node->order_specs[i].ascending ? " ASC" : " DESC");
            }
            std::cout << "]";
            break;
        case RANodeType::LIMIT_OP:
            std::cout << " [LIMIT " << node->limit_count;
            if (node->offset_count > 0)
                std::cout << " OFFSET " << node->offset_count;
            std::cout << "]";
            break;
        case RANodeType::GROUP_BY:
            std::cout << " [";
            for (size_t i = 0; i < node->group_by_exprs.size(); i++) {
                if (i > 0) std::cout << ", ";
                std::cout << expr_to_string(node->group_by_exprs[i]);
            }
            std::cout << "]";
            break;
        default:
            break;
    }
    std::cout << std::endl;

    // Print children
    bool has_left = node->left != nullptr;
    bool has_right = node->right != nullptr;

    if (has_left) {
        ra_tree_print_helper(node->left, child_prefix, !has_right, false);
    }
    if (has_right) {
        ra_tree_print_helper(node->right, child_prefix, true, false);
    }
}

inline void ra_tree_print(const RANodePtr& root) {
    std::cout << "\nRelational Algebra Tree:\n";
    ra_tree_print_helper(root, "", true, true);
}