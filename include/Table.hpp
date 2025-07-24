#pragma once

#include <Types.hpp>
#include <PageCache.hpp>
#include <vector>
#include <string>

struct Page {

};
enum Orientation {
    ROW,
    COLUMN
};

struct Column {
    string  name;
    u16     size;
};
struct Row {
    size_t      size;
    std::byte*  data;
};

class Table {
    public:
        //initialize new Table
        Table(const string& path, const std::vector<Column>& schema, PageCache& bufPool);

        // initialize existing Table
        Table(const string& path, PageCache& bufPool);

        //return page number where row is placed. row byte size must equal schema
        u64           insert(const Row& row);
        bool          read(u64 pageNum, u16 rowNum, Row row);

    private:
        Orientation         theOrientation;
        string              theName;
        size_t              theRowSize;
        std::vector<u64>    thePageNumbers;     
        PageCache&          thePageCache;

        void loadSchema(); //read first page
        void writeHeader_(); // write schema & metadata to page 0
        u64  allocPage_(); // grab a free page from file

}; 