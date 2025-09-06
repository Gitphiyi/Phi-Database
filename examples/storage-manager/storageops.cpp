#include "general/Structs.hpp"
#include "general/Types.hpp"
#include "storage-manager/Table.hpp"
#include "storage-manager/ops/StorageOps.hpp"
#include "page-manager/PageCache.hpp"

#include <iostream>

using namespace DB;

int main() {
    std::cout << "Running Table Example...\n";
    const string table_name = "example";
    auto schema = Schema(4);
    DbFile::initialize(true);
    auto cache = PageCache(10);
    const Table t = Table(table_name, schema, cache);
    SeqScan tableScan(t, 4);
    //tableScan.open();
    auto ret = tableScan.next();
    tableScan.print(ret);

    return 0;
}