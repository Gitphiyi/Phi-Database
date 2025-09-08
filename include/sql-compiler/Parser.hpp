#pragma once

#include "general/Types.hpp"
#include "general/AST.hpp"
#include "sql-compiler/SqlAST.hpp"
#include "sql-compiler/Types.hpp"
#include <vector>

namespace DB {
    std::vector<SqlNode>                 create_SQL_AST(std::vector<Token> tokens);             
}