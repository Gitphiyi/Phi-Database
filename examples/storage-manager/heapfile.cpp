#include "page-manager/DbFile.hpp"
#include "storage-manager/HeapFile.hpp"
#include "general/Structs.hpp"
#include <iostream>
using namespace DB;

int main() {
    std::cout << "Running Heapfile Example... \n";
    Schema schema = Schema(6); // 6 cols
    schema.add_col("id", ColumnType::INT64);
    schema.add_col("character", ColumnType::CHAR);
    schema.add_col("name", ColumnType::STRING);
    schema.print();

    string tablename = "table1";
    DbFile::initialize(false);
    auto h = HeapFile(1, tablename, false);

    return 0;
}