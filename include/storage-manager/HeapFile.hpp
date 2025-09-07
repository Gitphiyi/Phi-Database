#pragma once

#include "general/Page.hpp"
#include "storage-manager/StorageStructs.hpp"
#include "page-manager/PageCache.hpp"

namespace DB {
    struct HeapFile {
        Page*               getPage(Page pid); 
        std::vector<Row>    get_rows(RowId id); 
        PageCache cache;
    };
}
