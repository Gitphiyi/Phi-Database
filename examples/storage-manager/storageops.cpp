#include "general/Structs.hpp"
#include "general/Types.hpp"
#include "general/Comparison.hpp"
#include "storage-manager/Table.hpp"
#include "storage-manager/ops/StorageOps.hpp"
#include "storage-manager/ops/Selection.hpp"
#include "page-manager/PageCache.hpp"

#include <iostream>

using namespace DB;

void test_table_scan() {
    std::cout << "Running Table Scan...\n";
    const string table_name = "example";
    auto schema = Schema(4);
    DbFile::initialize(true);
    auto cache = PageCache(10);
    const Table t = Table(table_name, schema, cache);
    SeqScan tableScan(t, 4);
}

void test_naive_selection() {
    std::cout << "Running Naive Selection...\n";
    const string table_name = "example";
    auto schema = Schema(4);
    DbFile::initialize(true);
    auto cache = PageCache(10);
    const Table t = Table(table_name, schema, cache);
    SeqScan tableScan(t, 4);
    NaiveSelection selector(&tableScan, condfn_generator("<")); 
    auto res = selector.next();
    tableScan.print(res);
}

int main() {
    //test_table_scan();
    test_naive_selection();

    return 0;
}