#pragma once

#include "general/Page.hpp"
#include "storage-manager/StorageStructs.hpp"
#include "page-manager/PageCache.hpp"

namespace DB {
    /**
     * Only allow fixed sized pages
     * Page Directory implementation of heapfiles
     */
    struct HeapFile_Metadata {
        string identifier; //identifies it is a file of type heapfile
        u64    heap_id;
        TableId table_id;
        u64 num_pages;
        u64 num_records;
    };

    struct HeapFile {
        Page*               get_page(u32 pid); 
        //table specific functions
        //table just needs a way to get, insert, delete, update, and scan rows 
        Row                 get_row(RowId id); 
        RowId               insert_row(Row* row, u32 page);
        void                delete_row(RowId rid);
    };
}
