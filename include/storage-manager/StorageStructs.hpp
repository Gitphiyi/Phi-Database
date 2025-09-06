#pragma once

#include "general/Types.hpp"

struct RowId {
    u64     pageId;
    u64     slotId;
};

struct Row {
    u8          numCols;
    datatype*   values
};