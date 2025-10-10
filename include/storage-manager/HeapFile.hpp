#pragma once

#include "general/Page.hpp"
#include "storage-manager/StorageStructs.hpp"
#include "page-manager/PageCache.hpp"

#include <vector>
#include <stdint.h>
#include <iostream>

namespace DB {
    /**
     * Only allow fixed sized pages
     * Page Directory implementation of heapfiles
     */
    struct HeapFile_Metadata {
        string  identifier; // type heapfile and is always at page 0
        u64     heap_id; // local to table
        u64     table_id;
        u64     num_pages;
        u64     num_records;
        u8      next_heapfile; // 0 means null
        ssize_t size = identifier.size()+1+sizeof(heap_id)+sizeof(table_id)+sizeof(num_pages)
                        +sizeof(num_records)+sizeof(next_heapfile);
    };
    inline std::byte* to_bytes(HeapFile_Metadata* metadata) {
        std::byte* buffer = new std::byte[metadata->size];
        size_t offset = 0;

        // string contents
        std::memcpy(buffer + offset, metadata->identifier.data(), metadata->identifier.size() + 1);
        offset += metadata->identifier.size()+1;

        // integers
        std::memcpy(buffer + offset, &(metadata->heap_id), sizeof(metadata->heap_id));
        offset += sizeof(metadata->heap_id);

        std::memcpy(buffer + offset, &(metadata->table_id), sizeof(metadata->table_id));
        offset += sizeof(metadata->table_id);

        std::memcpy(buffer + offset, &(metadata->num_pages), sizeof(metadata->num_pages));
        offset += sizeof(metadata->num_pages);

        std::memcpy(buffer + offset, &(metadata->num_records), sizeof(metadata->num_records));
        offset += sizeof(metadata->num_records);
                std::memcpy(buffer + offset, &(metadata->next_heapfile), sizeof(metadata->next_heapfile));
        offset += sizeof(metadata->next_heapfile);

        return buffer; 
    }

    /**
     * dictionary to search up what pages are free and what is stored
     */
    struct HeapPageEntry {
        u32 page_id;
        u64 free_space; // number of free bytes
    };

    // First page is always heapfile metadata + list of page ids
    struct HeapFile {
        Page*               get_page(u32 pid); 
        //table specific functions
        //table just needs a way to get, insert, delete, update, and scan rows 
        // Row                 get_row(RowId id); 
        // RowId               insert_row(Row* row, u32 page);
        // void                delete_row(RowId rid);

        HeapFile_Metadata   metadata;
        int                 heap_fd;                 
        int                 num_heapfiles;
        HeapFile(int table_id, string tablename, bool if_missing);
    };

    HeapFile* create_heapfile(int table_id, string tablename, bool if_missing);
    HeapFile* initalize_heapfile(int table_id, string tablename, bool if_missing);
    HeapFile* initalize_heapfile(string tablename);
    HeapFile* read_heapfile();

    void print_heapfile(HeapFile heapfile);
}
