#pragma once 

#include "Types.hpp"
#include "Table.hpp"
#include "Logger.hpp"

#include <vector>
#include <stack>

namespace DB {
    enum Orientation {
        COLUMN,
        ROW
    };

    class Database {
        public:
            Database(string name);
            int     create_database();
            int     write_header_page();
            int     create_table(string tableName, Orientation type, const Schema& schema);
            string  print_tables();
            
        private:
            void                    create_header_page();
            Page                    theHeaderPage; //currently will just contain table info but should also contain info about how to shard, current capacity in db, etc.
            string                  theName;
            std::stack<int>         theFreePages; //stack of pages that are free. These pages can be from pages that get deleted or not from if Tables get deleted or not         
            std::vector<Table>      theTables; //Table objects that should create header table pages as well
            //Logger&                 theLogger;
            //PageCache&              thePageCache;
    };
}

//Database Header
// Table pointer