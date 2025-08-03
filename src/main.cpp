#include <iostream>
#include "Page.hpp"
#include "DbFile.hpp"
#include <sys/stat.h>

using namespace DB;
// bazel run --config=local_run src:main

int main(int argc, char** argv) {
    setup();
    const char* filename = "hi.db";
    std::cout << "Creating dbfile at: " << filename << std::endl;
    Page testPage{Page(-1)};
    DbFile dbFile = DbFile(filename, true, testPage);
    return 1;
}

void setup() {
    mkdir("db", 0755);
}