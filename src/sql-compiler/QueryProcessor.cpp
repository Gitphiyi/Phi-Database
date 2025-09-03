#include "sql-compiler/QueryProcessor.hpp"

#include <iostream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <string>

namespace DB {

    //creates AST from tokens
    void parse_syntax(std::vector<Token> tokens) {

    }

    //checks AST to make sure all table/cols exist, type checking, validate aggregates, etc.
    void analyze_semantics() {

    }

    //convert AST to relational algebra tree
    void convert_to_ra() {

    }

    // optionally optimize plan

    //hand off ra queries to table to be executed as table ops
}