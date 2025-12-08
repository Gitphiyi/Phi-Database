#pragma once

#include "general/Types.hpp"
#include "general/Page.hpp"
#include "general/Structs.hpp"
#include "storage-manager/StorageStructs.hpp"
#include "storage-manager/HeapFile.hpp"
#include <vector>
#include <string>

namespace DB {
    // Forward declarations for table operations
    void create_table();
    void load_table();
    void load_schema();

    class Table {
        public:
            // Initialize new Table
            Table(const string& name, Schema& schema, HeapFile& heapfile);

            // Get table metadata from disk
            static Table* get_table(const string& name, HeapFile& bufPool);

            // Row operations
            RowId               insert_row();
            Row*                read_row();
            std::vector<Row*>   scan() const;

            u64                 read(u64 pageNum, u16 rowNum);
            string              print_metadata();
            Schema*             get_record(int rid);

            // Accessors
            const string&       getName() const { return theFileName; }
            const Schema&       getSchema() const { return theSchema; }
            HeapFile*           getHeapFile() { return theHeapFile; }

        private:
            const string            theFileName;
            const string            thePath;
            std::vector<u64>        theFreePages;
            Schema                  theSchema;
            HeapFile*               theHeapFile;

            u64 allocPage(); // grab a free page from file
    };
}