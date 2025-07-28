#pragma once

#include "Types.hpp"
#include "PageCache.hpp"
#include "DbFile.hpp"
#include <vector>
#include <string>

using DataType = std::variant<int, float, std::string, bool, int64_t>;

enum Orientation {
    ROW,
    COLUMN
};

struct Column {
    string      name;
    DataType    type;
    bool        nullable;
    DataType    default_val;
    bool        is_primary_key;
    bool        is_unique;

    Column(const string& name, DataType type, bool nullable = true,
        bool primaryKey = false, bool unique = false, DataType defaultVal = {}) : 
        name(name),
        type(type),
        nullable(nullable),
        is_primary_key(primaryKey),
        is_unique(unique),
        default_val(defaultVal) {}
};
struct Row {
    size_t      size;
    std::byte*  data;
};

class Table {
    public:
        //initialize new Table
        Table(const string& filepath, const Orientation type, const std::vector<Column&>& schema, PageCache& cache);

        // initialize existing Table
        Table(const string& filepath, PageCache& bufPool);

        //return page number where row is placed. row byte size must equal schema
        u64           insert(const Row& row);
        u64           read(u64 pageNum, u16 rowNum, Row row);
        string        print_metadata();

    private:
        const Orientation       theOrientation;
        DB::DbFile              theDbFile;
        string                  theFilepath;
        size_t                  theRowSize;
        std::vector<u64>        theFreePages; 
        std::vector<Column>&    theSchema;    
        PageCache&              theCache;

        void loadSchema(); //read first page
        void writeHeader_(); // write schema & metadata to page 0
        u64  allocPage_(); // grab a free page from file

}; 