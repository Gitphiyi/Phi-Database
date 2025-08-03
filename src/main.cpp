#include <iostream>
#include <sys/stat.h>

#include "Types.hpp"
#include "Page.hpp"
#include "DbFile.hpp"
#include <cstddef>

using namespace DB;
// bazel run --config=local_run src:main

void setup() {
    mkdir("db", 0755);
}

int main(int argc, char** argv) {
    setup();
    const char* filename = "db/hi.db";
    std::cout << "Creating dbfile at: " << filename << std::endl;
    Page buffer{Page(-1)};
    DbFile dbFile = DbFile(filename, true, buffer);

    for(int i = 0; i < PAGE_SIZE; i++) {
        buffer.data[i] = static_cast<std::byte>('c');
    }
    buffer.print();

    return 1;
}