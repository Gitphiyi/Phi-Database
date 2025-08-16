#include "Types.hpp"
#include "Table.hpp"
#include "Page.hpp"
#include "DbFile.hpp"

#include <sys/fcntl.h>

using namespace DB;

Table::Table(const string& name,
             Orientation type, 
            Schema& schema, 
            PageCache& cache
            ) :
    theOrientation(type),
    theFileName(name),
    thePath("db/table/"+name),
    theSchema(schema),
    theCache(cache) {
        
    }



// u64 read(u64 pageNum, u16 rowNum, Row row) {
//     // first check buffer
//     // make a read to disk
// }

