#pragma once

#include "general/Page.hpp"
#include "storage-manager/StorageStructs.hpp"
#include "page-manager/PageCache.hpp"

namespace DB {
    struct HeapFile {
        Page*               get_page(u32 pid); 
        //table specific functions
        //table just needs a way to get, insert, delete, update, and scan rows 
        Row                 get_row(RowId id); 
        RowId               insert_row(Row* row, u32 page);
        void                delete_row(RowId rid);

        PageCache& cache;

    };
}
