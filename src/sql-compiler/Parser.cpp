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

bool Parser::isAtEnd() const {
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

RANodePtr Parser::parse() { return parseStatement(); }
RANodePtr Parser::parseStatement() {
  if (check("SELECT")) {
    return parseSelectStatement();
  } else if (check("INSERT")) {
    return parseInsertStatement();
  } else if (check("UPDATE")) {
    return parseUpdateStatement();
  } else if (check("DELETE")) {
    return parseDeleteStatement();
  } else if (check("CREATE")) {
    return parseCreateTableStatement();
  } else if (check("DROP")) {
    return parseDropTableStatement();
  }
  throw std::runtime_error("Unknown statement type: " + current().value);
}

// SELECT
RANodePtr Parser::parseSelectStatement() {
  consume("SELECT", "Expected SELECT");

  // parse cols
  SelectInfo select_info = parseSelectClause();
  // parse FROM clause
  RANodePtr from_node = parseFromClause();
  // parse optional WHERE clause
  ExprPtr where_predicate = nullptr;
  if (check("WHERE")) {
    where_predicate = parseWhereClause();
  }
  // parse optional GROUP BY clause
  std::vector<ExprPtr> group_by_exprs;
  if (check("GROUP")) {
    group_by_exprs = parseGroupByClause();
  }

  // parse optional HAVING clause
  ExprPtr having_predicate = nullptr;
  if (check("HAVING")) {
    having_predicate = parseHavingClause();
  }

  // parse optional ORDER BY clause
  std::vector<SortSpec> order_specs;
  if (check("ORDER")) {
    order_specs = parseOrderByClause();
  }

  // parse optional LIMIT clause
  int64_t limit_count = -1;
  int64_t offset_count = 0;
  if (check("LIMIT") || check("OFFSET")) {
    auto [limit, offset] = parseLimitClause();
    limit_count = limit;
    offset_count = offset;
  }

  // Build the RA tree bottom-up
  RANodePtr result = from_node;

  // Apply WHERE (σ - selection)
  if (where_predicate) {
    result = RANode::makeSelect(result, where_predicate);
  }

  // Apply GROUP BY (γ - grouping)
  if (!group_by_exprs.empty()) {
    result = RANode::makeGroupBy(result, std::move(group_by_exprs),
                                 having_predicate);
  }

  // Apply PROJECT (π - projection)
  auto project = std::make_shared<RANode>(RANodeType::PROJECT);
  project->left = result;
  project->select_all = select_info.select_all;
  project->projections = std::move(select_info.projections);
  project->projection_aliases = std::move(select_info.aliases);
  project->is_distinct = select_info.is_distinct;
  result = project;

  // Apply DISTINCT if needed (and not already in project node)
  if (select_info.is_distinct) {
    result = RANode::makeDistinct(result);
  }

  // Apply ORDER BY (τ - sort)
  if (!order_specs.empty()) {
    result = RANode::makeSort(result, std::move(order_specs));
  }

  // Apply LIMIT/OFFSET
  if (limit_count >= 0 || offset_count > 0) {
    result = RANode::makeLimit(result, limit_count, offset_count);
  }

  return result;
}

Parser::SelectInfo Parser::parseSelectClause() {
  SelectInfo info;

  // Check for DISTINCT or ALL
  if (match("DISTINCT")) {
    info.is_distinct = true;
  } else if (match("ALL")) {
    info.is_distinct = false;
  }

  // Check for SELECT *
  if (check("*") && (peek(1).value == "FROM" || peek(1).value == ",")) {
    if (match("*")) {
      info.select_all = true;
      // Could have more columns after *
      if (!check("FROM") && match(",")) {
        // Parse additional columns
      } else {
        return info;
      }
    }
  }

  // Parse column list
  do {
    if (check("*")) {
      advance();
      info.select_all = true;
      info.projections.push_back(nullptr); // Placeholder for *
      info.aliases.push_back("");
    } else {
      ExprPtr expr = parseExpression();
      string alias;

      // Check for AS alias
      if (match("AS")) {
        alias = consume(IDENTIFIER, "Expected alias after AS").value;
      } else if (check(IDENTIFIER) && !check("FROM") && !check(",")) {
        // Implicit alias (without AS)
        alias = current().value;
        advance();
      }

      info.projections.push_back(expr);
      info.aliases.push_back(alias);
    }
  } while (match(","));

  return info;
}

// ============================================================================
// FROM Clause
// ============================================================================

RANodePtr Parser::parseFromClause() {
  consume("FROM", "Expected FROM");

  RANodePtr result = parseTableReference();

  // Parse JOINs
  while (check("JOIN") || check("INNER") || check("LEFT") || check("RIGHT") ||
         check("FULL") || check("CROSS") || check("NATURAL") || check(",")) {
    result = parseJoinClause(result);
  }

  return result;
}

RANodePtr Parser::parseTableReference() {
  // Check for subquery
  if (check("(")) {
    advance(); // consume '('
    if (check("SELECT")) {
      RANodePtr subquery = parseSelectStatement();
      consume(")", "Expected ')' after subquery");

      // Subqueries usually need an alias
      string alias;
      if (match("AS")) {
        alias = consume(IDENTIFIER, "Expected alias after AS").value;
      } else if (check(IDENTIFIER)) {
        alias = current().value;
        advance();
      }

      // Wrap in a rename node if alias provided
      if (!alias.empty()) {
        auto rename = std::make_shared<RANode>(RANodeType::RENAME);
        rename->left = subquery;
        rename->table_alias = alias;
        return rename;
      }
      return subquery;
    }
    // Could be a parenthesized table reference
    RANodePtr inner = parseTableReference();
    consume(")", "Expected ')'");
    return inner;
  }

  // Simple table reference
  string table_name = consume(IDENTIFIER, "Expected table name").value;
  string alias = table_name;

  // Check for alias
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

RANodePtr Parser::parseJoinClause(RANodePtr left) {
  RANodeType join_type = RANodeType::INNER_JOIN;
  bool is_natural = false;

  // Handle comma as cross join
  if (match(",")) {
    RANodePtr right = parseTableReference();
    return RANode::makeJoin(RANodeType::CROSS_PRODUCT, left, right, nullptr);
  }

  // Determine join type
  if (match("NATURAL")) {
    is_natural = true;
  }

  if (match("CROSS")) {
    join_type = RANodeType::CROSS_PRODUCT;
  } else if (match("LEFT")) {
    match("OUTER"); // OUTER is optional
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

  RANodePtr right = parseTableReference();

  // Parse ON condition (not for CROSS JOIN or NATURAL JOIN)
  ExprPtr condition = nullptr;
  if (join_type != RANodeType::CROSS_PRODUCT && !is_natural) {
    if (match("ON")) {
      condition = parseExpression();
    } else if (match("USING")) {
      // USING (col1, col2, ...)
      consume("(", "Expected '(' after USING");
      std::vector<string> using_cols;
      do {
        using_cols.push_back(consume(IDENTIFIER, "Expected column name").value);
      } while (match(","));
      consume(")", "Expected ')' after USING columns");

      // Convert USING to ON condition
      // USING(a, b) => left.a = right.a AND left.b = right.b
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

// ============================================================================
// WHERE Clause
// ============================================================================

ExprPtr Parser::parseWhereClause() {
  consume("WHERE", "Expected WHERE");
  return parseExpression();
}

// ============================================================================
// GROUP BY Clause
// ============================================================================

std::vector<ExprPtr> Parser::parseGroupByClause() {
  consume("GROUP", "Expected GROUP");
  consume("BY", "Expected BY after GROUP");

  std::vector<ExprPtr> exprs;
  do {
    exprs.push_back(parseExpression());
  } while (match(","));

  return exprs;
}

// ============================================================================
// HAVING Clause
// ============================================================================

ExprPtr Parser::parseHavingClause() {
  consume("HAVING", "Expected HAVING");
  return parseExpression();
}

// ============================================================================
// ORDER BY Clause
// ============================================================================

std::vector<SortSpec> Parser::parseOrderByClause() {
  consume("ORDER", "Expected ORDER");
  consume("BY", "Expected BY after ORDER");

  std::vector<SortSpec> specs;
  do {
    SortSpec spec;

    // Could be table.column or just column
    string first = consume(IDENTIFIER, "Expected column name").value;
    if (match(".")) {
      spec.table = first;
      spec.column = consume(IDENTIFIER, "Expected column name after '.'").value;
    } else {
      spec.column = first;
    }

    // ASC or DESC
    if (match("ASC")) {
      spec.ascending = true;
    } else if (match("DESC")) {
      spec.ascending = false;
    } else {
      spec.ascending = true; // Default is ASC
    }

    // NULLS FIRST or NULLS LAST
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

// ============================================================================
// LIMIT Clause
// ============================================================================

std::pair<int64_t, int64_t> Parser::parseLimitClause() {
  int64_t limit = -1;
  int64_t offset = 0;

  if (match("LIMIT")) {
    limit = std::stoll(consume(NUMBER, "Expected number after LIMIT").value);

    // LIMIT x, y syntax (MySQL style: LIMIT offset, count)
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

// ============================================================================
// Expression Parsing (Precedence Climbing)
// ============================================================================

ExprPtr Parser::parseExpression() { return parseOrExpression(); }

ExprPtr Parser::parseOrExpression() {
  ExprPtr left = parseAndExpression();

  while (match("OR")) {
    ExprPtr right = parseAndExpression();
    left = Expression::makeBinaryOp(BinaryOp::OR, left, right);
  }

  return left;
}

ExprPtr Parser::parseAndExpression() {
  ExprPtr left = parseNotExpression();

  while (match("AND")) {
    ExprPtr right = parseNotExpression();
    left = Expression::makeBinaryOp(BinaryOp::AND, left, right);
  }

  return left;
}

ExprPtr Parser::parseNotExpression() {
  if (match("NOT")) {
    ExprPtr operand = parseNotExpression();
    return Expression::makeUnaryOp(UnaryOp::NOT, operand);
  }
  return parseComparisonExpression();
}

ExprPtr Parser::parseComparisonExpression() {
  ExprPtr left = parseAddSubExpression();

  // Check for IN
  if (check("IN") || (check("NOT") && peek(1).value == "IN")) {
    return parseInExpression(left);
  }

  // Check for BETWEEN
  if (check("BETWEEN") || (check("NOT") && peek(1).value == "BETWEEN")) {
    return parseBetweenExpression(left);
  }

  // Check for IS NULL / IS NOT NULL
  if (match("IS")) {
    bool is_not = match("NOT");
    consume("NULL", "Expected NULL after IS");
    return Expression::makeUnaryOp(
        is_not ? UnaryOp::IS_NOT_NULL : UnaryOp::IS_NULL, left);
  }

  // Check for LIKE
  if (match("LIKE")) {
    ExprPtr pattern = parseAddSubExpression();
    return Expression::makeBinaryOp(BinaryOp::LIKE, left, pattern);
  }

  // Regular comparison operators
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
      return left; // Not a comparison operator

    advance();
    ExprPtr right = parseAddSubExpression();
    return Expression::makeBinaryOp(bin_op, left, right);
  }

  return left;
}

ExprPtr Parser::parseAddSubExpression() {
  ExprPtr left = parseMulDivExpression();

  while (check(OPERATOR) && (current().value == "+" || current().value == "-" ||
                             current().value == "||")) {
    string op = current().value;
    advance();
    ExprPtr right = parseMulDivExpression();

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

ExprPtr Parser::parseMulDivExpression() {
  ExprPtr left = parseUnaryExpression();

  while (check(OPERATOR) && (current().value == "*" || current().value == "/" ||
                             current().value == "%")) {
    string op = current().value;
    advance();
    ExprPtr right = parseUnaryExpression();

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

ExprPtr Parser::parseUnaryExpression() {
  if (check(OPERATOR) && (current().value == "-" || current().value == "+")) {
    string op = current().value;
    advance();
    ExprPtr operand = parseUnaryExpression();
    return Expression::makeUnaryOp(op == "-" ? UnaryOp::MINUS : UnaryOp::PLUS,
                                   operand);
  }
  return parsePrimaryExpression();
}

ExprPtr Parser::parsePrimaryExpression() {
  // Parenthesized expression or subquery
  if (match("(")) {
    if (check("SELECT")) {
      // Subquery
      RANodePtr subquery = parseSelectStatement();
      consume(")", "Expected ')' after subquery");
      auto expr = std::make_shared<Expression>();
      expr->type = ExprType::SUBQUERY;
      // Note: we'd need to store the subquery somehow
      return expr;
    }
    ExprPtr expr = parseExpression();
    consume(")", "Expected ')'");
    return expr;
  }

  // NULL literal
  if (match("NULL")) {
    return Expression::makeLiteralNull();
  }

  // Boolean literals
  if (match("TRUE")) {
    return Expression::makeLiteralBool(true);
  }
  if (match("FALSE")) {
    return Expression::makeLiteralBool(false);
  }

  // Number literal
  if (check(NUMBER)) {
    string num = current().value;
    advance();
    if (num.find('.') != string::npos) {
      return Expression::makeLiteralFloat(std::stod(num));
    }
    return Expression::makeLiteralInt(std::stoll(num));
  }

  // String literal
  if (check(STRING)) {
    string str = current().value;
    advance();
    return Expression::makeLiteralString(str);
  }

  // CASE expression
  if (check("CASE")) {
    return parseCaseExpression();
  }

  // EXISTS
  if (match("EXISTS")) {
    consume("(", "Expected '(' after EXISTS");
    RANodePtr subquery = parseSelectStatement();
    consume(")", "Expected ')' after subquery");
    auto expr = std::make_shared<Expression>();
    expr->type = ExprType::EXISTS;
    return expr;
  }

  // Column reference or function call
  return parseColumnOrFunction();
}

ExprPtr Parser::parseCaseExpression() {
  consume("CASE", "Expected CASE");

  auto expr = std::make_shared<Expression>();
  expr->type = ExprType::CASE_WHEN;

  // Simple CASE or searched CASE
  ExprPtr case_operand = nullptr;
  if (!check("WHEN")) {
    case_operand = parseExpression();
  }

  while (match("WHEN")) {
    ExprPtr when_expr = parseExpression();
    consume("THEN", "Expected THEN");
    ExprPtr then_expr = parseExpression();
    expr->children.push_back(when_expr);
    expr->children.push_back(then_expr);
  }

  if (match("ELSE")) {
    ExprPtr else_expr = parseExpression();
    expr->children.push_back(else_expr);
  }

  consume("END", "Expected END");
  return expr;
}

ExprPtr Parser::parseColumnOrFunction() {
  if (!check(IDENTIFIER)) {
    throw std::runtime_error("Expected identifier, got '" + current().value +
                             "'");
  }

  string first = current().value;
  advance();

  // Check for function call
  if (check("(")) {
    return parseFunctionCall(first);
  }

  // Check for table.column
  if (match(".")) {
    // Handle special case of table.*
    if (match("*")) {
      auto expr = Expression::makeColumnRef("*", first);
      return expr;
    }
    string column = consume(IDENTIFIER, "Expected column name after '.'").value;
    return Expression::makeColumnRef(column, first);
  }

  // Just a column name
  return Expression::makeColumnRef(first);
}

ExprPtr Parser::parseFunctionCall(const string &name) {
  consume("(", "Expected '(' for function call");

  std::vector<ExprPtr> args;

  // Handle COUNT(*) specially
  if (name == "COUNT" && check("*")) {
    advance();
    args.push_back(Expression::makeColumnRef("*"));
  } else if (!check(")")) {
    // Check for DISTINCT in aggregate functions
    bool is_distinct = match("DISTINCT");

    do {
      args.push_back(parseExpression());
    } while (match(","));

    if (is_distinct && !args.empty()) {
      // Mark the first arg as distinct somehow
      // This is a simplification
    }
  }

  consume(")", "Expected ')' after function arguments");

  return Expression::makeFunctionCall(name, std::move(args));
}

ExprPtr Parser::parseInExpression(ExprPtr left) {
  bool is_not = match("NOT");
  consume("IN", "Expected IN");
  consume("(", "Expected '(' after IN");

  auto expr = std::make_shared<Expression>();
  expr->type = ExprType::IN_LIST;
  expr->children.push_back(left);

  if (check("SELECT")) {
    // Subquery
    RANodePtr subquery = parseSelectStatement();
    // Would need to handle subquery storage
  } else {
    // Value list
    do {
      expr->in_list.push_back(parseExpression());
    } while (match(","));
  }

  consume(")", "Expected ')' after IN list");

  if (is_not) {
    return Expression::makeUnaryOp(UnaryOp::NOT, expr);
  }
  return expr;
}

ExprPtr Parser::parseBetweenExpression(ExprPtr left) {
  bool is_not = match("NOT");
  consume("BETWEEN", "Expected BETWEEN");

  ExprPtr low = parseAddSubExpression();
  consume("AND", "Expected AND in BETWEEN");
  ExprPtr high = parseAddSubExpression();

  // BETWEEN is equivalent to: left >= low AND left <= high
  auto ge = Expression::makeBinaryOp(BinaryOp::GE, left, low);
  auto le = Expression::makeBinaryOp(BinaryOp::LE, left, high);
  auto result = Expression::makeBinaryOp(BinaryOp::AND, ge, le);

  if (is_not) {
    return Expression::makeUnaryOp(UnaryOp::NOT, result);
  }
  return result;
}

std::vector<ExprPtr> Parser::parseExpressionList() {
  std::vector<ExprPtr> exprs;
  do {
    exprs.push_back(parseExpression());
  } while (match(","));
  return exprs;
}

// ============================================================================
// INSERT Statement
// ============================================================================

RANodePtr Parser::parseInsertStatement() {
  consume("INSERT", "Expected INSERT");
  consume("INTO", "Expected INTO after INSERT");

  string table_name = consume(IDENTIFIER, "Expected table name").value;

  auto node = std::make_shared<RANode>(RANodeType::INSERT_OP);
  node->table_name = table_name;

  // Optional column list
  if (match("(")) {
    do {
      node->insert_columns.push_back(
          consume(IDENTIFIER, "Expected column name").value);
    } while (match(","));
    consume(")", "Expected ')' after column list");
  }

  consume("VALUES", "Expected VALUES");
  node->insert_values = parseValuesList();

  return node;
}

std::vector<std::vector<ExprPtr>> Parser::parseValuesList() {
  std::vector<std::vector<ExprPtr>> rows;

  do {
    consume("(", "Expected '(' for VALUES");
    std::vector<ExprPtr> row;
    do {
      row.push_back(parseExpression());
    } while (match(","));
    consume(")", "Expected ')' after values");
    rows.push_back(std::move(row));
  } while (match(","));

  return rows;
}

// ============================================================================
// UPDATE Statement
// ============================================================================

RANodePtr Parser::parseUpdateStatement() {
  consume("UPDATE", "Expected UPDATE");

  string table_name = consume(IDENTIFIER, "Expected table name").value;

  auto node = std::make_shared<RANode>(RANodeType::UPDATE_OP);
  node->table_name = table_name;

  consume("SET", "Expected SET");

  // Parse assignments
  do {
    string column = consume(IDENTIFIER, "Expected column name").value;
    consume("=", "Expected '=' in SET clause");
    ExprPtr value = parseExpression();
    node->update_assignments.push_back({column, value});
  } while (match(","));

  // Optional WHERE clause
  if (check("WHERE")) {
    node->predicate = parseWhereClause();
  }

  return node;
}

// ============================================================================
// DELETE Statement
// ============================================================================

RANodePtr Parser::parseDeleteStatement() {
  consume("DELETE", "Expected DELETE");
  consume("FROM", "Expected FROM after DELETE");

  string table_name = consume(IDENTIFIER, "Expected table name").value;

  auto node = std::make_shared<RANode>(RANodeType::DELETE_OP);
  node->table_name = table_name;

  // Optional WHERE clause
  if (check("WHERE")) {
    node->predicate = parseWhereClause();
  }

  return node;
}

// ============================================================================
// CREATE TABLE Statement
// ============================================================================

RANodePtr Parser::parseCreateTableStatement() {
  consume("CREATE", "Expected CREATE");
  consume("TABLE", "Expected TABLE");

  string table_name = consume(IDENTIFIER, "Expected table name").value;

  auto node = std::make_shared<RANode>(RANodeType::CREATE_TABLE_OP);
  node->table_name = table_name;

  consume("(", "Expected '(' after table name");
  node->column_defs = parseColumnDefinitions();
  consume(")", "Expected ')' after column definitions");

  return node;
}

std::vector<ColumnDef> Parser::parseColumnDefinitions() {
  std::vector<ColumnDef> defs;

  do {
    ColumnDef def;
    def.name = consume(IDENTIFIER, "Expected column name").value;
    def.type_name = consume(IDENTIFIER, "Expected type name").value;

    // Optional type parameters like VARCHAR(255)
    if (match("(")) {
      // Skip parameters for now
      while (!check(")") && !isAtEnd()) {
        advance();
      }
      consume(")", "Expected ')' after type parameters");
    }

    // Column constraints
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
        def.default_value = parseExpression();
      } else if (match("UNIQUE")) {
        // Handle unique constraint
      } else if (match("CHECK")) {
        consume("(", "Expected '(' after CHECK");
        parseExpression(); // Discard for now
        consume(")", "Expected ')' after CHECK expression");
      } else {
        break;
      }
    }

    defs.push_back(def);
  } while (match(","));

  return defs;
}

// ============================================================================
// DROP TABLE Statement
// ============================================================================

RANodePtr Parser::parseDropTableStatement() {
  consume("DROP", "Expected DROP");
  consume("TABLE", "Expected TABLE");

  auto node = std::make_shared<RANode>(RANodeType::DROP_TABLE_OP);
  node->table_name = consume(IDENTIFIER, "Expected table name").value;

  return node;
}

// ============================================================================
// Legacy API Implementation
// ============================================================================

void sql_query(SqlNode *root, std::vector<Token> &tokens,
               std::vector<string> &aliases, int st) {
  // Legacy implementation - kept for compatibility but deprecated
  std::cout << "WARNING: Using legacy sql_query. Use parse_to_ra() instead.\n";
}

std::vector<SqlNode> create_SQL_AST(std::vector<Token> tokens) {
  // Legacy implementation - kept for compatibility
  std::vector<SqlNode> result;
  result.emplace_back(SqlNode("SELECT"));

  // Try new parser and print RA tree
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
