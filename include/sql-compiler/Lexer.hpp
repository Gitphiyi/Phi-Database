#pragma once

#include "general/Types.hpp"
#include "sql-compiler/Types.hpp"
#include <vector>

namespace DB {
    std::vector<Token>  tokenize_query(string& query);
}
// void                read_next();

