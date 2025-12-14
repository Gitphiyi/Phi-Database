#pragma once

#include "general/Types.hpp"
#include "sql-compiler/SqlAST.hpp"
#include "sql-compiler/Types.hpp"
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace DB {

class Parser {
public:
  Parser(const std::vector<Token> &tokens);

  RANodePtr parse();

private:
  std::vector<Token> tokens_;
  size_t pos_ = 0;

  const Token &current() const;
  const Token &peek(int offset = 0) const;
  bool is_at_end() const;
  bool check(const string &value) const;
  bool check(TokenType type) const;
  bool match(const string &value);
  bool match(TokenType type);
  Token consume(const string &value, const string &error_msg);
  Token consume(TokenType type, const string &error_msg);
  void advance();

  RANodePtr parse_statement();
  RANodePtr parse_select_statement();
  RANodePtr parse_insert_statement();
  RANodePtr parse_update_statement();
  RANodePtr parse_delete_statement();
  RANodePtr parse_create_table_statement();
  RANodePtr parse_drop_table_statement();

  struct SelectInfo {
    bool is_distinct = false;
    bool select_all = false;
    std::vector<ExprPtr> projections;
    std::vector<string> aliases;
  };
  SelectInfo parse_select_clause();

  RANodePtr parse_from_clause();
  RANodePtr parse_table_reference();
  RANodePtr parse_join_clause(RANodePtr left);

  ExprPtr parse_where_clause();

  std::vector<ExprPtr> parse_group_by_clause();

  ExprPtr parse_having_clause();

  std::vector<SortSpec> parse_order_by_clause();

  std::pair<int64_t, int64_t> parse_limit_clause();

  ExprPtr parse_expression();
  ExprPtr parse_or_expression();
  ExprPtr parse_and_expression();
  ExprPtr parse_not_expression();
  ExprPtr parse_comparison_expression();
  ExprPtr parse_add_sub_expression();
  ExprPtr parse_mul_div_expression();
  ExprPtr parse_unary_expression();
  ExprPtr parse_primary_expression();
  ExprPtr parse_column_or_function();
  ExprPtr parse_function_call(const string &name);
  ExprPtr parse_in_expression(ExprPtr left);
  ExprPtr parse_between_expression(ExprPtr left);
  ExprPtr parse_case_expression();

  std::vector<ExprPtr> parse_expression_list();

  std::vector<std::vector<ExprPtr>> parse_values_list();

  std::vector<ColumnDef> parse_column_definitions();
};

using queryFunc = void (*)(SqlNode *, std::vector<Token> &,
                           std::vector<string> &, int);

void sql_query(SqlNode *root, std::vector<Token> &tokens,
               std::vector<string> &stmt_aliases, int st);

static std::unordered_map<string, queryFunc> start_tokens = {
    {"SELECT", &sql_query}};

std::vector<SqlNode> create_sql_ast(std::vector<Token> tokens);

RANodePtr parse_to_ra(std::vector<Token> &tokens);

RANodePtr compile_sql(string &query);

} // namespace DB
