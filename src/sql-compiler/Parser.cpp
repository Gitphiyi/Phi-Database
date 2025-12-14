#include "sql-compiler/Parser.hpp"
#include "general/Types.hpp"
#include "sql-compiler/Lexer.hpp"

#include <cstdio>
#include <iostream>
#include <stdexcept>

namespace DB {
Parser::Parser(const std::vector<Token> &tokens) : tokens_(tokens), pos_(0) {}

const Token &Parser::current() const {
  static Token eof{KEYWORD, ""};
  if (pos_ >= tokens_.size())
    return eof;
  return tokens_[pos_];
}

const Token &Parser::peek(int offset) const {
  static Token eof{KEYWORD, ""};
  size_t idx = pos_ + offset;
  if (idx >= tokens_.size())
    return eof;
  return tokens_[idx];
}

bool Parser::is_at_end() const {
  return pos_ >= tokens_.size() || current().value == ";";
}

bool Parser::check(const string &value) const {
  if (pos_ >= tokens_.size())
    return false;
  return current().value == value;
}

bool Parser::check(TokenType type) const {
  if (pos_ >= tokens_.size())
    return false;
  return current().type == type;
}

bool Parser::match(const string &value) {
  if (check(value)) {
    advance();
    return true;
  }
  return false;
}

bool Parser::match(TokenType type) {
  if (check(type)) {
    advance();
    return true;
  }
  return false;
}

Token Parser::consume(const string &value, const string &error_msg) {
  if (check(value)) {
    Token t = current();
    advance();
    return t;
  }
  throw std::runtime_error(error_msg + " (got '" + current().value + "')");
}

Token Parser::consume(TokenType type, const string &error_msg) {
  if (check(type)) {
    Token t = current();
    advance();
    return t;
  }
  throw std::runtime_error(error_msg + " (got '" + current().value + "')");
}

void Parser::advance() {
  if (pos_ < tokens_.size())
    pos_++;
}

RANodePtr Parser::parse() {
  if (check("SELECT")) {
    return parse_select_statement();
  } else if (check("INSERT")) {
    return parse_insert_statement();
  } else if (check("UPDATE")) {
    return parse_update_statement();
  } else if (check("DELETE")) {
    return parse_delete_statement();
  } else if (check("CREATE")) {
    return parse_create_table_statement();
  } else if (check("DROP")) {
    return parse_drop_table_statement();
  }
  throw std::runtime_error("Unknown statement type: " + current().value);
}

RANodePtr Parser::parse_select_statement() {
  consume("SELECT", "Expected SELECT");

  SelectInfo select_info = parse_select_clause();
  RANodePtr from_node = parse_from_clause();
  ExprPtr where_predicate = nullptr;
  if (check("WHERE")) {
    where_predicate = parse_where_clause();
  }
  std::vector<ExprPtr> group_by_exprs;
  if (check("GROUP")) {
    group_by_exprs = parse_group_by_clause();
  }

  ExprPtr having_predicate = nullptr;
  if (check("HAVING")) {
    having_predicate = parse_having_clause();
  }

  std::vector<SortSpec> order_specs;
  if (check("ORDER")) {
    order_specs = parse_order_by_clause();
  }

  int64_t limit_count = -1;
  int64_t offset_count = 0;
  if (check("LIMIT") || check("OFFSET")) {
    auto [limit, offset] = parse_limit_clause();
    limit_count = limit;
    offset_count = offset;
  }

  RANodePtr result = from_node;

  if (where_predicate) {
    result = RANode::makeSelect(result, where_predicate);
  }

  if (!group_by_exprs.empty()) {
    result = RANode::makeGroupBy(result, std::move(group_by_exprs),
                                 having_predicate);
  }

  auto project = std::make_shared<RANode>(RANodeType::PROJECT);
  project->left = result;
  project->select_all = select_info.select_all;
  project->projections = std::move(select_info.projections);
  project->projection_aliases = std::move(select_info.aliases);
  project->is_distinct = select_info.is_distinct;
  result = project;

  if (select_info.is_distinct) {
    result = RANode::makeDistinct(result);
  }

  if (!order_specs.empty()) {
    result = RANode::makeSort(result, std::move(order_specs));
  }

  if (limit_count >= 0 || offset_count > 0) {
    result = RANode::makeLimit(result, limit_count, offset_count);
  }

  return result;
}

Parser::SelectInfo Parser::parse_select_clause() {
  SelectInfo info;

  if (match("DISTINCT")) {
    info.is_distinct = true;
  } else if (match("ALL")) {
    info.is_distinct = false;
  }

  if (check("*") && (peek(1).value == "FROM" || peek(1).value == ",")) {
    if (match("*")) {
      info.select_all = true;
      if (!check("FROM") && match(",")) {
      } else {
        return info;
      }
    }
  }

  do {
    if (check("*")) {
      advance();
      info.select_all = true;
      info.projections.push_back(nullptr);
      info.aliases.push_back("");
    } else {
      ExprPtr expr = parse_expression();
      string alias;

      if (match("AS")) {
        alias = consume(IDENTIFIER, "Expected alias after AS").value;
      } else if (check(IDENTIFIER) && !check("FROM") && !check(",")) {
        alias = current().value;
        advance();
      }

      info.projections.push_back(expr);
      info.aliases.push_back(alias);
    }
  } while (match(","));

  return info;
}

RANodePtr Parser::parse_from_clause() {
  consume("FROM", "Expected FROM");

  RANodePtr result = parse_table_reference();

  while (check("JOIN") || check("INNER") || check("LEFT") || check("RIGHT") ||
         check("FULL") || check("CROSS") || check("NATURAL") || check(",")) {
    result = parse_join_clause(result);
  }

  return result;
}

RANodePtr Parser::parse_table_reference() {
  if (check("(")) {
    advance();
    if (check("SELECT")) {
      RANodePtr subquery = parse_select_statement();
      consume(")", "Expected ')' after subquery");

      string alias;
      if (match("AS")) {
        alias = consume(IDENTIFIER, "Expected alias after AS").value;
      } else if (check(IDENTIFIER)) {
        alias = current().value;
        advance();
      }

      if (!alias.empty()) {
        auto rename = std::make_shared<RANode>(RANodeType::RENAME);
        rename->left = subquery;
        rename->table_alias = alias;
        return rename;
      }
      return subquery;
    }
    RANodePtr inner = parse_table_reference();
    consume(")", "Expected ')'");
    return inner;
  }

  string table_name = consume(IDENTIFIER, "Expected table name").value;
  string alias = table_name;

  if (match("AS")) {
    alias = consume(IDENTIFIER, "Expected alias after AS").value;
  } else if (check(IDENTIFIER) && !check("JOIN") && !check("INNER") &&
             !check("LEFT") && !check("RIGHT") && !check("FULL") &&
             !check("CROSS") && !check("NATURAL") && !check("ON") &&
             !check("WHERE") && !check("GROUP") && !check("ORDER") &&
             !check("LIMIT") && !check(",") && !check(";")) {
    alias = current().value;
    advance();
  }

  return RANode::makeTableScan(table_name, alias);
}

RANodePtr Parser::parse_join_clause(RANodePtr left) {
  RANodeType join_type = RANodeType::INNER_JOIN;
  bool is_natural = false;

  if (match(",")) {
    RANodePtr right = parse_table_reference();
    return RANode::makeJoin(RANodeType::CROSS_PRODUCT, left, right, nullptr);
  }

  if (match("NATURAL")) {
    is_natural = true;
  }

  if (match("CROSS")) {
    join_type = RANodeType::CROSS_PRODUCT;
  } else if (match("LEFT")) {
    match("OUTER");
    join_type = RANodeType::LEFT_JOIN;
  } else if (match("RIGHT")) {
    match("OUTER");
    join_type = RANodeType::RIGHT_JOIN;
  } else if (match("FULL")) {
    match("OUTER");
    join_type = RANodeType::FULL_JOIN;
  } else if (match("INNER")) {
    join_type = RANodeType::INNER_JOIN;
  }

  consume("JOIN", "Expected JOIN");

  RANodePtr right = parse_table_reference();

  ExprPtr condition = nullptr;
  if (join_type != RANodeType::CROSS_PRODUCT && !is_natural) {
    if (match("ON")) {
      condition = parse_expression();
    } else if (match("USING")) {
      consume("(", "Expected '(' after USING");
      std::vector<string> using_cols;
      do {
        using_cols.push_back(consume(IDENTIFIER, "Expected column name").value);
      } while (match(","));
      consume(")", "Expected ')' after USING columns");

      for (size_t i = 0; i < using_cols.size(); i++) {
        auto left_col = Expression::makeColumnRef(using_cols[i]);
        auto right_col = Expression::makeColumnRef(using_cols[i]);
        auto eq = Expression::makeBinaryOp(BinaryOp::EQ, left_col, right_col);

        if (condition) {
          condition = Expression::makeBinaryOp(BinaryOp::AND, condition, eq);
        } else {
          condition = eq;
        }
      }
    }
  }

  return RANode::makeJoin(join_type, left, right, condition);
}

ExprPtr Parser::parse_where_clause() {
  consume("WHERE", "Expected WHERE");
  return parse_expression();
}

std::vector<ExprPtr> Parser::parse_group_by_clause() {
  consume("GROUP", "Expected GROUP");
  consume("BY", "Expected BY after GROUP");

  std::vector<ExprPtr> exprs;
  do {
    exprs.push_back(parse_expression());
  } while (match(","));

  return exprs;
}

ExprPtr Parser::parse_having_clause() {
  consume("HAVING", "Expected HAVING");
  return parse_expression();
}

std::vector<SortSpec> Parser::parse_order_by_clause() {
  consume("ORDER", "Expected ORDER");
  consume("BY", "Expected BY after ORDER");

  std::vector<SortSpec> specs;
  do {
    SortSpec spec;

    string first = consume(IDENTIFIER, "Expected column name").value;
    if (match(".")) {
      spec.table = first;
      spec.column = consume(IDENTIFIER, "Expected column name after '.'").value;
    } else {
      spec.column = first;
    }

    if (match("ASC")) {
      spec.ascending = true;
    } else if (match("DESC")) {
      spec.ascending = false;
    } else {
      spec.ascending = true;
    }

    if (match("NULLS")) {
      if (match("FIRST")) {
        spec.nulls_first = true;
      } else if (match("LAST")) {
        spec.nulls_first = false;
      }
    }

    specs.push_back(spec);
  } while (match(","));

  return specs;
}

std::pair<int64_t, int64_t> Parser::parse_limit_clause() {
  int64_t limit = -1;
  int64_t offset = 0;

  if (match("LIMIT")) {
    limit = std::stoll(consume(NUMBER, "Expected number after LIMIT").value);

    if (match(",")) {
      offset = limit;
      limit = std::stoll(consume(NUMBER, "Expected number after ','").value);
    }
  }

  if (match("OFFSET")) {
    offset = std::stoll(consume(NUMBER, "Expected number after OFFSET").value);
  }

  return {limit, offset};
}

ExprPtr Parser::parse_expression() { return parse_or_expression(); }

ExprPtr Parser::parse_or_expression() {
  ExprPtr left = parse_and_expression();

  while (match("OR")) {
    ExprPtr right = parse_and_expression();
    left = Expression::makeBinaryOp(BinaryOp::OR, left, right);
  }

  return left;
}

ExprPtr Parser::parse_and_expression() {
  ExprPtr left = parse_not_expression();

  while (match("AND")) {
    ExprPtr right = parse_not_expression();
    left = Expression::makeBinaryOp(BinaryOp::AND, left, right);
  }

  return left;
}

ExprPtr Parser::parse_not_expression() {
  if (match("NOT")) {
    ExprPtr operand = parse_not_expression();
    return Expression::makeUnaryOp(UnaryOp::NOT, operand);
  }
  return parse_comparison_expression();
}

ExprPtr Parser::parse_comparison_expression() {
  ExprPtr left = parse_add_sub_expression();

  if (check("IN") || (check("NOT") && peek(1).value == "IN")) {
    return parse_in_expression(left);
  }

  if (check("BETWEEN") || (check("NOT") && peek(1).value == "BETWEEN")) {
    return parse_between_expression(left);
  }

  if (match("IS")) {
    bool is_not = match("NOT");
    consume("NULL", "Expected NULL after IS");
    return Expression::makeUnaryOp(
        is_not ? UnaryOp::IS_NOT_NULL : UnaryOp::IS_NULL, left);
  }

  if (match("LIKE")) {
    ExprPtr pattern = parse_add_sub_expression();
    return Expression::makeBinaryOp(BinaryOp::LIKE, left, pattern);
  }

  if (check(OPERATOR)) {
    string op = current().value;
    BinaryOp bin_op;

    if (op == "=")
      bin_op = BinaryOp::EQ;
    else if (op == "<>" || op == "!=")
      bin_op = BinaryOp::NE;
    else if (op == "<")
      bin_op = BinaryOp::LT;
    else if (op == "<=")
      bin_op = BinaryOp::LE;
    else if (op == ">")
      bin_op = BinaryOp::GT;
    else if (op == ">=")
      bin_op = BinaryOp::GE;
    else
      return left;

    advance();
    ExprPtr right = parse_add_sub_expression();
    return Expression::makeBinaryOp(bin_op, left, right);
  }

  return left;
}

ExprPtr Parser::parse_add_sub_expression() {
  ExprPtr left = parse_mul_div_expression();

  while (check(OPERATOR) && (current().value == "+" || current().value == "-" ||
                             current().value == "||")) {
    string op = current().value;
    advance();
    ExprPtr right = parse_mul_div_expression();

    BinaryOp bin_op;
    if (op == "+")
      bin_op = BinaryOp::ADD;
    else if (op == "-")
      bin_op = BinaryOp::SUB;
    else
      bin_op = BinaryOp::CONCAT;

    left = Expression::makeBinaryOp(bin_op, left, right);
  }

  return left;
}

ExprPtr Parser::parse_mul_div_expression() {
  ExprPtr left = parse_unary_expression();

  while (check(OPERATOR) && (current().value == "*" || current().value == "/" ||
                             current().value == "%")) {
    string op = current().value;
    advance();
    ExprPtr right = parse_unary_expression();

    BinaryOp bin_op;
    if (op == "*")
      bin_op = BinaryOp::MUL;
    else if (op == "/")
      bin_op = BinaryOp::DIV;
    else
      bin_op = BinaryOp::MOD;

    left = Expression::makeBinaryOp(bin_op, left, right);
  }

  return left;
}

ExprPtr Parser::parse_unary_expression() {
  if (check(OPERATOR) && (current().value == "-" || current().value == "+")) {
    string op = current().value;
    advance();
    ExprPtr operand = parse_unary_expression();
    return Expression::makeUnaryOp(op == "-" ? UnaryOp::MINUS : UnaryOp::PLUS,
                                   operand);
  }
  return parse_primary_expression();
}

ExprPtr Parser::parse_primary_expression() {
  if (match("(")) {
    if (check("SELECT")) {
      RANodePtr subquery = parse_select_statement();
      consume(")", "Expected ')' after subquery");
      auto expr = std::make_shared<Expression>();
      expr->type = ExprType::SUBQUERY;
      return expr;
    }
    ExprPtr expr = parse_expression();
    consume(")", "Expected ')'");
    return expr;
  }

  if (match("NULL")) {
    return Expression::makeLiteralNull();
  }

  if (match("TRUE")) {
    return Expression::makeLiteralBool(true);
  }
  if (match("FALSE")) {
    return Expression::makeLiteralBool(false);
  }

  if (check(NUMBER)) {
    string num = current().value;
    advance();
    if (num.find('.') != string::npos) {
      return Expression::makeLiteralFloat(std::stod(num));
    }
    return Expression::makeLiteralInt(std::stoll(num));
  }

  if (check(STRING)) {
    string str = current().value;
    advance();
    return Expression::makeLiteralString(str);
  }

  if (check("CASE")) {
    return parse_case_expression();
  }

  if (match("EXISTS")) {
    consume("(", "Expected '(' after EXISTS");
    RANodePtr subquery = parse_select_statement();
    consume(")", "Expected ')' after subquery");
    auto expr = std::make_shared<Expression>();
    expr->type = ExprType::EXISTS;
    return expr;
  }

  return parse_column_or_function();
}

ExprPtr Parser::parse_case_expression() {
  consume("CASE", "Expected CASE");

  auto expr = std::make_shared<Expression>();
  expr->type = ExprType::CASE_WHEN;

  ExprPtr case_operand = nullptr;
  if (!check("WHEN")) {
    case_operand = parse_expression();
  }

  while (match("WHEN")) {
    ExprPtr when_expr = parse_expression();
    consume("THEN", "Expected THEN");
    ExprPtr then_expr = parse_expression();
    expr->children.push_back(when_expr);
    expr->children.push_back(then_expr);
  }

  if (match("ELSE")) {
    ExprPtr else_expr = parse_expression();
    expr->children.push_back(else_expr);
  }

  consume("END", "Expected END");
  return expr;
}

ExprPtr Parser::parse_column_or_function() {
  if (!check(IDENTIFIER)) {
    throw std::runtime_error("Expected identifier, got '" + current().value +
                             "'");
  }

  string first = current().value;
  advance();

  if (check("(")) {
    return parse_function_call(first);
  }

  if (match(".")) {
    if (match("*")) {
      auto expr = Expression::makeColumnRef("*", first);
      return expr;
    }
    string column = consume(IDENTIFIER, "Expected column name after '.'").value;
    return Expression::makeColumnRef(column, first);
  }

  return Expression::makeColumnRef(first);
}

ExprPtr Parser::parse_function_call(const string &name) {
  consume("(", "Expected '(' for function call");

  std::vector<ExprPtr> args;

  if (name == "COUNT" && check("*")) {
    advance();
    args.push_back(Expression::makeColumnRef("*"));
  } else if (!check(")")) {
    bool is_distinct = match("DISTINCT");

    do {
      args.push_back(parse_expression());
    } while (match(","));

    (void)is_distinct;
  }

  consume(")", "Expected ')' after function arguments");

  return Expression::makeFunctionCall(name, std::move(args));
}

ExprPtr Parser::parse_in_expression(ExprPtr left) {
  bool is_not = match("NOT");
  consume("IN", "Expected IN");
  consume("(", "Expected '(' after IN");

  auto expr = std::make_shared<Expression>();
  expr->type = ExprType::IN_LIST;
  expr->children.push_back(left);

  if (check("SELECT")) {
    RANodePtr subquery = parse_select_statement();
  } else {
    do {
      expr->in_list.push_back(parse_expression());
    } while (match(","));
  }

  consume(")", "Expected ')' after IN list");

  if (is_not) {
    return Expression::makeUnaryOp(UnaryOp::NOT, expr);
  }
  return expr;
}

ExprPtr Parser::parse_between_expression(ExprPtr left) {
  bool is_not = match("NOT");
  consume("BETWEEN", "Expected BETWEEN");

  ExprPtr low = parse_add_sub_expression();
  consume("AND", "Expected AND in BETWEEN");
  ExprPtr high = parse_add_sub_expression();

  auto ge = Expression::makeBinaryOp(BinaryOp::GE, left, low);
  auto le = Expression::makeBinaryOp(BinaryOp::LE, left, high);
  auto result = Expression::makeBinaryOp(BinaryOp::AND, ge, le);

  if (is_not) {
    return Expression::makeUnaryOp(UnaryOp::NOT, result);
  }
  return result;
}

std::vector<ExprPtr> Parser::parse_expression_list() {
  std::vector<ExprPtr> exprs;
  do {
    exprs.push_back(parse_expression());
  } while (match(","));
  return exprs;
}

RANodePtr Parser::parse_insert_statement() {
  consume("INSERT", "Expected INSERT");
  consume("INTO", "Expected INTO after INSERT");

  string table_name = consume(IDENTIFIER, "Expected table name").value;

  auto node = std::make_shared<RANode>(RANodeType::INSERT_OP);
  node->table_name = table_name;

  if (match("(")) {
    do {
      node->insert_columns.push_back(
          consume(IDENTIFIER, "Expected column name").value);
    } while (match(","));
    consume(")", "Expected ')' after column list");
  }

  consume("VALUES", "Expected VALUES");
  node->insert_values = parse_values_list();

  return node;
}

std::vector<std::vector<ExprPtr>> Parser::parse_values_list() {
  std::vector<std::vector<ExprPtr>> rows;

  do {
    consume("(", "Expected '(' for VALUES");
    std::vector<ExprPtr> row;
    do {
      row.push_back(parse_expression());
    } while (match(","));
    consume(")", "Expected ')' after values");
    rows.push_back(std::move(row));
  } while (match(","));

  return rows;
}

RANodePtr Parser::parse_update_statement() {
  consume("UPDATE", "Expected UPDATE");

  string table_name = consume(IDENTIFIER, "Expected table name").value;

  auto node = std::make_shared<RANode>(RANodeType::UPDATE_OP);
  node->table_name = table_name;

  consume("SET", "Expected SET");

  do {
    string column = consume(IDENTIFIER, "Expected column name").value;
    consume("=", "Expected '=' in SET clause");
    ExprPtr value = parse_expression();
    node->update_assignments.push_back({column, value});
  } while (match(","));

  if (check("WHERE")) {
    node->predicate = parse_where_clause();
  }

  return node;
}

RANodePtr Parser::parse_delete_statement() {
  consume("DELETE", "Expected DELETE");
  consume("FROM", "Expected FROM after DELETE");

  string table_name = consume(IDENTIFIER, "Expected table name").value;

  auto node = std::make_shared<RANode>(RANodeType::DELETE_OP);
  node->table_name = table_name;

  if (check("WHERE")) {
    node->predicate = parse_where_clause();
  }

  return node;
}

RANodePtr Parser::parse_create_table_statement() {
  consume("CREATE", "Expected CREATE");
  consume("TABLE", "Expected TABLE");

  string table_name = consume(IDENTIFIER, "Expected table name").value;

  auto node = std::make_shared<RANode>(RANodeType::CREATE_TABLE_OP);
  node->table_name = table_name;

  consume("(", "Expected '(' after table name");
  node->column_defs = parse_column_definitions();
  consume(")", "Expected ')' after column definitions");

  return node;
}

std::vector<ColumnDef> Parser::parse_column_definitions() {
  std::vector<ColumnDef> defs;

  do {
    ColumnDef def;
    def.name = consume(IDENTIFIER, "Expected column name").value;
    def.type_name = consume(IDENTIFIER, "Expected type name").value;

    if (match("(")) {
      while (!check(")") && !is_at_end()) {
        advance();
      }
      consume(")", "Expected ')' after type parameters");
    }

    while (true) {
      if (match("NOT")) {
        consume("NULL", "Expected NULL after NOT");
        def.nullable = false;
      } else if (match("NULL")) {
        def.nullable = true;
      } else if (match("PRIMARY")) {
        consume("KEY", "Expected KEY after PRIMARY");
        def.primary_key = true;
        def.nullable = false;
      } else if (match("DEFAULT")) {
        def.default_value = parse_expression();
      } else if (match("UNIQUE")) {
      } else if (match("CHECK")) {
        consume("(", "Expected '(' after CHECK");
        parse_expression();
        consume(")", "Expected ')' after CHECK expression");
      } else {
        break;
      }
    }

    defs.push_back(def);
  } while (match(","));

  return defs;
}

RANodePtr Parser::parse_drop_table_statement() {
  consume("DROP", "Expected DROP");
  consume("TABLE", "Expected TABLE");

  auto node = std::make_shared<RANode>(RANodeType::DROP_TABLE_OP);
  node->table_name = consume(IDENTIFIER, "Expected table name").value;

  return node;
}

void sql_query(SqlNode *root, std::vector<Token> &tokens,
               std::vector<string> &aliases, int st) {
  std::cout << "WARNING: Using legacy sql_query. Use parse_to_ra() instead.\n";
}

std::vector<SqlNode> create_sql_ast(std::vector<Token> tokens) {
  std::vector<SqlNode> result;
  result.emplace_back(SqlNode("SELECT"));

  try {
    Parser parser(tokens);
    RANodePtr ra = parser.parse();
    ra_tree_print(ra);
  } catch (const std::exception &e) {
    std::cerr << "Parse error: " << e.what() << std::endl;
  }

  return result;
}

RANodePtr parse_to_ra(std::vector<Token> &tokens) {
  Parser parser(tokens);
  return parser.parse();
}

RANodePtr compile_sql(string &query) {
  std::vector<Token> tokens = tokenize_query(query);
  return parse_to_ra(tokens);
}

} // namespace DB
