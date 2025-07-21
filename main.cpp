#include "include/StorageInterface.hpp"
#include <iostream>

int main() {
    const char* filename = "hi.db";
    std::cout << " hi\n";
    create_file(filename);

    return 1;
}