#include "storage-manager/ops/StorageOps.hpp"

#include <iostream>
#include <iomanip>

namespace DB {
    void StorageOps::print(std::vector<Row*>& rows) {
        const int colWidth = 12;
        const int numCols = rows[0]->numCols;
        std::cout << std::string((colWidth+1) * numCols, '-') << "\n";
        for (Row* r : rows) {
            for(int i = 0; i < r->numCols; i++) {
                std::visit([&](auto&& val) {
                    std::cout << std::setw(colWidth) << std::left << val;
                }, r->values[i]);
                std::cout << "|";
            }
            std::cout<< std::endl;
        }
        std::cout << std::string((colWidth+1) * numCols, '-') << "\n";

    }

    SeqScan::SeqScan(const Table& table, size_t batchSize = 64) : table(table), batchSize(batchSize), cursor(0) {}
    void SeqScan::open() {
        cursor = 0;
    }
    std::vector<Row*> SeqScan::next() {
        return table.scan();
    }
    void SeqScan::close() {
        std::cout << "Closing Scan on Table";
    }
}