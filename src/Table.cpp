#include "Types.hpp"
#include "Table.hpp"
#include "Page.hpp"
#include "DbFile.hpp"

#include <sys/fcntl.h>


Table::Table(const string& name, const Orientation type, const std::vector<Column>& schema, PageCache& cache) :
    theFileName("db/table/"+name),
    theOrientation(type),
    theCache(cache),
    theRowSize(computeRowSize(schema))
{}



// u64 read(u64 pageNum, u16 rowNum, Row row) {
//     // first check buffer
//     // make a read to disk
// }

