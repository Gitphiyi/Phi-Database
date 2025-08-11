#include <iostream>
#include <cstddef>
#include <sys/stat.h>

#include "Types.hpp"
#include "Page.hpp"
#include "DbFile.hpp"
#include "Database.hpp"
#include "PageCache.hpp"

using namespace DB;


void dbfile_test(DbFile& dbFile, Page* buffer) {
    buffer->valid_bit = 1;
    for(int i = 0; i < PAGE_SIZE; i += sizeof(int)) {
        memcpy(buffer->data + i, &i, sizeof(int));
    }

    dbFile.db_write_at(0, *buffer);
    dbFile.read_at(buffer->id, *buffer, 0);
    
    buffer->print<int>();

    dbFile.close();
}

void pagecache_test(PageCache& cache, const string filepath) {
    Page* test = new Page(1);
    Page* test_read = new Page(2);
    for(int i = 0; i < PAGE_DATA_SIZE / sizeof(int); i ++) {
        memcpy(test->data + sizeof(int) * i, &i, sizeof(int));
    }
    cache.write_through(*test, filepath);
    cache.read(test->id, *test_read, filepath);
    test_read->print<int>();
    delete test;
    delete test_read;
}

int main(int argc, char** argv) {
    const string filename = "db/hi.db";
    mkdir("db", 0755);
    mkdir("db/table", 0755);
    std::cout << "Creating dbfile at: " << filename << std::endl;
    u32 numPages = 10;
    DbFile dbFile = DbFile(filename, true);
    PageCache pgCache(numPages, dbFile);
    Database db("phi-db");

    //dbfile_test();
    pagecache_test(pgCache, filename);

    return 0;
}