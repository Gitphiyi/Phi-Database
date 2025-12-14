#pragma once

#include "general/Types.hpp"
#include "general/Page.hpp"
#include "general/Structs.hpp"
#include "storage-manager/StorageStructs.hpp"
#include "storage-manager/HeapFile.hpp"
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace DB {
    void create_table();
    void load_table();
    void load_schema();

    class Table {
        public:
            Table(const string& name, Schema& schema, HeapFile& heapfile);

            static Table* get_table(const string& name, HeapFile& bufPool);

            RowId               insert_row();
            RowId               insert_row(Row* row);
            Row*                read_row();
            Row*                read_row(const RowId& rid);
            std::vector<Row*>   scan() const;

            u64                 read(u64 pageNum, u16 rowNum);
            string              print_metadata();
            Schema*             get_record(int rid);

            const string&       getName() const { return theFileName; }
            const Schema&       getSchema() const { return theSchema; }
            HeapFile*           getHeapFile() { return theHeapFile; }

        private:
            const string            theFileName;
            const string            thePath;
            std::vector<u64>        theFreePages;
            Schema                  theSchema;
            HeapFile*               theHeapFile;

            u64 allocPage();
    };
}