#include "storage-manager/Table.hpp"

#include <sys/fcntl.h>

namespace DB {
    Table::Table(const string& name,
            Schema& schema, 
            HeapFile& heapfile
            ) :
        theFileName(name),
        thePath("db/table/"+name),
        theSchema(schema)
        {}

    std::vector<Row*> Table::scan() const {
        std::vector<Row*> batch;
        for (int i = 0; i < 100; ++i) {
            string s = "hi";
            std::vector<datatype> data = {s, false, i, 2.0};
            Row* r = new Row(4, std::move(data));
            batch.push_back(r);
        }
        return batch;
    }
    // std::vector<Row*> join() const {

    // }
}

