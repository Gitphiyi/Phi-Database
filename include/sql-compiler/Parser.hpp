#pragma once

#include "general/Types.hpp"
#include "sql-compiler/SqlAST.hpp"
#include "sql-compiler/Types.hpp"
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <memory>

namespace DB {

// ============================================================================
// SQL Parser - Recursive Descent Parser that produces RA Trees
// ============================================================================

class Parser {
public:
    Parser(const std::vector<Token>& tokens);

    // Main entry point - parses and returns an RA tree
    RANodePtr parse();

private:
    std::vector<Token> tokens_;
    size_t pos_ = 0;

    // Token access
    const Token& current() const;
    const Token& peek(int offset = 0) const;
    bool isAtEnd() const;
    bool check(const string& value) const;
    bool check(TokenType type) const;
    bool match(const string& value);
    bool match(TokenType type);
    Token consume(const string& value, const string& error_msg);
    Token consume(TokenType type, const string& error_msg);
    void advance();

    // Statement parsing
    RANodePtr parseStatement();
    RANodePtr parseSelectStatement();
    RANodePtr parseInsertStatement();
    RANodePtr parseUpdateStatement();
    RANodePtr parseDeleteStatement();
    RANodePtr parseCreateTableStatement();
    RANodePtr parseDropTableStatement();

    // SELECT clause components
    struct SelectInfo {
        bool is_distinct = false;
        bool select_all = false;
        std::vector<ExprPtr> projections;
        std::vector<string> aliases;
    };
    SelectInfo parseSelectClause();

    // FROM clause - returns the RA subtree for table sources
    RANodePtr parseFromClause();
    RANodePtr parseTableReference();
    RANodePtr parseJoinClause(RANodePtr left);

    // WHERE clause
    ExprPtr parseWhereClause();

    // GROUP BY clause
    std::vector<ExprPtr> parseGroupByClause();

    // HAVING clause
    ExprPtr parseHavingClause();

    // ORDER BY clause
    std::vector<SortSpec> parseOrderByClause();

    // LIMIT clause
    std::pair<int64_t, int64_t> parseLimitClause();  // (limit, offset)

    // Expression parsing (for WHERE, ON, HAVING, etc.)
    ExprPtr parseExpression();
    ExprPtr parseOrExpression();
    ExprPtr parseAndExpression();
    ExprPtr parseNotExpression();
    ExprPtr parseComparisonExpression();
    ExprPtr parseAddSubExpression();
    ExprPtr parseMulDivExpression();
    ExprPtr parseUnaryExpression();
    ExprPtr parsePrimaryExpression();
    ExprPtr parseColumnOrFunction();
    ExprPtr parseFunctionCall(const string& name);
    ExprPtr parseInExpression(ExprPtr left);
    ExprPtr parseBetweenExpression(ExprPtr left);
    ExprPtr parseCaseExpression();

    // Helper to parse comma-separated expressions
    std::vector<ExprPtr> parseExpressionList();

    // For INSERT VALUES
    std::vector<std::vector<ExprPtr>> parseValuesList();

    // Column definitions for CREATE TABLE
    std::vector<ColumnDef> parseColumnDefinitions();
};

// ============================================================================
// Legacy API (kept for compatibility)
// ============================================================================

using queryFunc = void(*)(SqlNode*, std::vector<Token>&, std::vector<string>&, int);

void sql_query(SqlNode* root, std::vector<Token>& tokens, std::vector<string>& stmt_aliases, int st);

static std::unordered_map<string, queryFunc> start_tokens = {
    {"SELECT", &sql_query}
};

// Legacy function - now wraps the new parser
std::vector<SqlNode> create_SQL_AST(std::vector<Token> tokens);

// New API - returns RA tree directly
RANodePtr parse_to_ra(std::vector<Token>& tokens);

// Convenience function - tokenize and parse in one call
RANodePtr compile_sql(string& query);

}