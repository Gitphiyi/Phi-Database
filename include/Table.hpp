#pragma once

#include "Types.hpp"
#include "PageCache.hpp"
#include "DbFile.hpp"
#include <vector>
#include <string>

namespace DB {
    class Table {
        public:
            //initialize new Table
            Table(const string& name, const Orientation type, Schema& schema);
            //destructor for  removing table FD and closing pipe

            // initialize existing Table
            //Table(const string& name, PageCache& bufPool);

            //return page number where row is placed. row byte size must equal schema
            u64             insert();
            void            write_page(u64 pageNum);
            u64             read(u64 pageNum, u16 rowNum);
            string          print_metadata();

        private:
            const Orientation       theOrientation;
            const string            theFileName;
            size_t                  theRowSize;
            std::vector<u64>        theFreePages; 
            Schema                  theSchema;    
            PageCache               theCache;

            void loadSchema(); //read first page
            void writeHeader(); // write schema & metadata to page 0
            u64  allocPage(); // grab a free page from file

    }; 
}