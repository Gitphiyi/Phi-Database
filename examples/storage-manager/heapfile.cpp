#include "storage-manager/HeapFile.hpp"
#include "general/Structs.hpp"
#include <iostream>

int main() {
    std::cout << "Running Heapfile Example... \n";
    Schema schema = Schema(6); // 6 cols
    schema.add_col("id", ColumnType::INT64);
    schema.add_col("character", ColumnType::CHAR);
    schema.add_col("name", ColumnType::STRING);
    schema.print();
    return 0;
}