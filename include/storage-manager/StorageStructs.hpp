#pragma once

#include "general/Types.hpp"
#include <vector>

struct RowId {
    u64     pageId;
    u64     slotId;
};

struct Row {
    //RowId       id; // for lazy allocation when needed
    u8          numCols;
    std::vector<datatype>   values;
    Row(int n, std::vector<datatype>&& v) : numCols(n), values(std::move(v)) {}
};
