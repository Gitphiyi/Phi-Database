#include "../include/Table.hpp"
#include <sys/fcntl.h>


Table::Table(const string& filepath, const Orientation type, const std::vector<Column>& schema, PageCache& cache) :
    theFilepath(filepath),
    theDbFile(DbFile(filepath, false, 0666)),
    theOrientation(type),
    theCache(cache),
    theRowSize(computeRowSize(schema))
{
    theFreePages.clear();
}

static std::size_t computeRowSize(const std::vector<Column>& schema) {
    std::size_t size = 0;
    for (auto& col : schema) {
        size += col.size;
    }
    return size;
}