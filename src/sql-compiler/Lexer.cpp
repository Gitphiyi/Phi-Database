#include "sql-compiler/Lexer.hpp"
#include <cctype>
#include <iostream>

namespace DB {
std::vector<Token> tokenize_query(string &query) {
  std::vector<Token> tokens;

  // Convert query to uppercase in place
  std::transform(query.begin(), query.end(), query.begin(),
                 [](unsigned char c) { return std::toupper(c); });

  size_t i = 0;
  while (i < query.length()) {
    char c = query[i];

    // Skip whitespace
    if (isspace(c)) {
      i++;
      continue;
    }

    // Skip single-line comments (-- style)
    if (c == '-' && i + 1 < query.length() && query[i + 1] == '-') {
      while (i < query.length() && query[i] != '\n') {
        i++;
      }
      continue;
    }

    // Skip multi-line comments
    if (c == '/' && i + 1 < query.length() && query[i + 1] == '*') {
      i += 2;
      while (i + 1 < query.length() &&
             !(query[i] == '*' && query[i + 1] == '/')) {
        i++;
      }
      i += 2; // Skip */
      continue;
    }

    // Identifier/Keyword
    if (isalpha(c) || c == '_') {
      size_t start = i;
      while (i < query.length() && (isalnum(query[i]) || query[i] == '_')) {
        i++;
      }
      string str = query.substr(start, i - start);
      if (keywords.contains(str)) {
        tokens.push_back(Token{KEYWORD, str});
      } else {
        tokens.push_back(Token{IDENTIFIER, str});
      }
      continue;
    }

    if (isdigit(c)) {
      size_t start = i;
      while (i < query.length() && isdigit(query[i])) {
        i++;
      }
      if (i < query.length() && query[i] == '.') {
        i++;
        while (i < query.length() && isdigit(query[i])) {
          i++;
        }
      }
      tokens.push_back(Token{NUMBER, query.substr(start, i - start)});
      continue;
    }

    if (c == '\'') {
      i++;
      string str_val;
      while (i < query.size()) {
        if (query[i] == '\'' && i + 1 < query.size() && query[i + 1] == '\'') {
          str_val += '\'';
          i += 2;
        } else if (query[i] == '\'') {
          break;
        } else {
          str_val += query[i];
          i++;
        }
      }
      tokens.push_back(Token{STRING, str_val});
      if (i < query.size())
        i++;
      continue;
    }

    if (c == '"') {
      i++;
      size_t start = i;
      while (i < query.size() && query[i] != '"') {
        i++;
      }
      tokens.push_back(Token{IDENTIFIER, query.substr(start, i - start)});
      if (i < query.size())
        i++;
      continue;
    }

    if (i + 1 < query.length()) {
      string two_char = query.substr(i, 2);
      if (two_char == "<>" || two_char == "<=" || two_char == ">=" ||
          two_char == "!=" || two_char == "::" || two_char == "||") {
        tokens.push_back(Token{OPERATOR, two_char});
        i += 2;
        continue;
      }
    }

    // Symbols
    if (symbols.contains(string(1, c))) {
      tokens.push_back(Token{SYMBOL, std::string(1, c)});
      i++;
      continue;
    }

    // Single-character operators
    if (sql_ops.contains(string(1, c))) {
      tokens.push_back(Token{OPERATOR, std::string(1, c)});
      i++;
      continue;
    }

    throw std::runtime_error("Unexpected character in SQL: " + string(1, c));
  }
  return tokens;
}
} // namespace DB
