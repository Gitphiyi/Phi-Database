#include <iostream>
#include <cstddef>
#include <sys/stat.h>

#include "Types.hpp"
#include "Page.hpp"
#include "DbFile.hpp"

using namespace DB;

void setup() {
    mkdir("db", 0755);
}

void dbfile_test(const char* filename) {
    std::cout << "Creating dbfile at: " << filename << std::endl;
    Page* buffer = new Page();
    Page read{Page()};
    DbFile dbFile = DbFile(filename, true, *buffer);

    buffer->valid_bit = 1;
    for(int i = 0; i < PAGE_SIZE; i+= sizeof(int)) {
        memcpy(buffer->data + i, &i, sizeof(int));
    }

    dbFile.write_at(0);
    dbFile.read_at(*buffer, 0);
    
    buffer->print<int>();

    dbFile.close();
}

int main(int argc, char** argv) {
    const char* filename = "db/hi.db";
    setup();
    dbfile_test(filename);

    return 1;
}