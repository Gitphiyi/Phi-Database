#include <iostream>
#include <cstddef>
#include <sys/stat.h>

#include "Types.hpp"
#include "Page.hpp"
#include "DbFile.hpp"
#include "PageCache.hpp"

using namespace DB;


void dbfile_test(DbFile& dbfile) {
    buffer->valid_bit = 1;
    for(int i = 0; i < PAGE_SIZE; i += sizeof(int)) {
        memcpy(buffer->data + i, &i, sizeof(int));
    }

    dbFile.write_at(0);
    dbFile.read_at(*buffer, 0);
    
    buffer->print<int>();

    dbFile.close();
}

void pagecache_test() {
}

int main(int argc, char** argv) {
    const char* filename = "db/hi.db";
    mkdir("db", 0755);
    std::cout << "Creating dbfile at: " << filename << std::endl;
    Page* buffer = new Page();
    DbFile dbfile = DbFile(filename, true, *buffer);
    //PageCache cache = new PageCache(10, dbfile);
    //dbfile_test();
    //pagecache_test();
    delete buffer;

    return 1;
}