#include "storage-manager/Table.hpp"

#include <sys/fcntl.h>

using namespace DB;

Table::Table(const string& name,
        Schema& schema, 
        PageCache& cache
        ) :
    theFileName(name),
    thePath("db/table/"+name),
    theSchema(schema),
    theCache(cache) {
    DbFile& file_writer = DbFile::getInstance();
    // file_writer.add_path(thePath);   
    // Page* metadata = new Page(0);
    // char* data_addr = reinterpret_cast<char*>(metadata->data);
    // TableMetadataHeader myHeader(name, schema);

    // memcpy(data_addr, &myHeader, myHeader.get_size());
    // theCache.write_through(*metadata, theFileName);
    // delete metadata;
}
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





// u64 read(u64 pageNum, u16 rowNum, Row row) {
//     // first check buffer
//     // make a read to disk
// }

