#pragma once

#include "Types.hpp"
#include "Table.hpp"
#include "Logger.hpp"
#include <vector>

class Database {
    public:
        Database(u64 pgSize, Logger& logger, PageCache& cache);
        int     create_database();
        int     write_metadata();
        int     create_table(string tableName, Orientation type, const std::vector<Column>& schema);
        string  print_tables();
        
    private:
        string                  theName;

        u64                     thePageSize;
        std::vector<Table&>     theTables;
        Logger&                 theLogger;
        PageCache&              thePageCache;


};
// +-------------------+  ← Offset 0
// | Header / Metadata |
// +-------------------+
// | Schema Definitions|
// +-------------------+
// | Free Page Bitmap  |
// +-------------------+
// | Data Page 1       |
// | Data Page 2       |
// | ...               |
// +-------------------+
// | WAL / Journaling  |  ← optional
// +-------------------+
