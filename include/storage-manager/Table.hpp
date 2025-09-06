#pragma once

#include "general/Types.hpp"
#include "general/Page.hpp"
#include "general/Structs.hpp"
#include "storage-manager/StorageStructs.hpp"
#include "page-manager/PageCache.hpp"
#include "page-manager/DbFile.hpp"
#include <vector>
#include <string>

namespace DB {
    class Table {
        public:
            //initialize new Table
            Table(const string& name, Schema& schema, PageCache& cache);
            //destructor for  removing table FD and closing pipe
            Table*          get_table(const string& name, PageCache& bufPool); //get table metadata from disk
            //return page number where row is placed. row byte size must equal schema
            RowId               insert_row();
            Row*                read_row();
            std::vector<Row>    table_scan();

            // void            write_page(u64 pageNum);
            u64             read(u64 pageNum, u16 rowNum);
            string          print_metadata();
            Schema*         get_record(int rid); //takes record id and returns the record

        private:
            const string            theFileName;
            const string            thePath;
            //bool                  theIndex;
            std::vector<u64>        theFreePages; 
            Schema                  theSchema;    
            PageCache               theCache;
            void loadSchema(); //read first page
            void writeHeader(); // write schema & metadata to page 0
            u64  allocPage(); // grab a free page from file

    }; 
}