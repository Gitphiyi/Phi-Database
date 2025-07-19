#include "PersistentStorage.hpp"
#include <iostream>

int main() {
    char* filename = "hi.db";
    std::cout << " hi\n";
    create_file(filename);
    return 1;
}