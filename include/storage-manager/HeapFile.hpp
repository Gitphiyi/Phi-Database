#pragma once

#include "general/Page.hpp"
#include "storage-manager/StorageStructs.hpp"
#include "page-manager/PageCache.hpp"

#include <vector>

namespace DB {
    /**
     * Only allow fixed sized pages
     * Page Directory implementation of heapfiles
     */
    struct HeapFile_Metadata {
        string identifier; //identifies it is a file of type heapfile
        u64    heap_id; // local to table
        u64 table_id;
        u64 num_pages;
        u64 num_records;
        ssize_t size = identifier.size()+sizeof(heap_id)+sizeof(table_id)+sizeof(num_pages)+sizeof(num_records); 
        std::byte* to_bytes() {
            std::byte* buffer = new std::byte[size];
            size_t offset = 0;

            // string contents
            std::memcpy(buffer + offset, identifier.data(), identifier.size());
            offset += identifier.size();

            // integers
            std::memcpy(buffer + offset, &heap_id, sizeof(heap_id));
            offset += sizeof(heap_id);

            std::memcpy(buffer + offset, &table_id, sizeof(table_id));
            offset += sizeof(table_id);

            std::memcpy(buffer + offset, &num_pages, sizeof(num_pages));
            offset += sizeof(num_pages);

            std::memcpy(buffer + offset, &num_records, sizeof(num_records));
            offset += sizeof(num_records);

            return buffer; 
        }

    };

    struct HeapFile {
        Page*               get_page(u32 pid); 
        //table specific functions
        //table just needs a way to get, insert, delete, update, and scan rows 
        static void         initialize(bool if_missing);
        Row                 get_row(RowId id); 
        RowId               insert_row(Row* row, u32 page);
        void                delete_row(RowId rid);

        HeapFile_Metadata   metadata;
        int                 num_heapfiles;
        std::vector<int>    heap_fd;                 


        HeapFile(int table_id, string tablename, bool if_missing);
        
    };
}
