#pragma once

#include "Table.hpp"

#include <tuple>

enum class RAOps {
    Selection,
    Projection,
    Renaming,
    Union,
    Difference,
    Intersection,
    Cross_Product,
    Join,
    Natural_Join
};

// Vectorized model of Operations
// Operations act on the return of next() from other operations
struct StorageOps {
    virtual ~StorageOps() = default;
    //set to 0 means pure virtual function
    virtual open() = 0; // initializes resources
    virtual next() = 0; // produces block of tuples (size of cacheline and does operation on it)
    virtual close() = 0 ;// closes all the resources
}

struct TableScan : StorageOps {
    TableScan();
    open(Table* table) override;
}

class Selection : StorageOps {
    public: 
        Selection();
        open(StorageOps* op) override;
    private:
        StorageOps*             childOp;
        std::function<bool>     condition;
}

struct Join final : StorageOps {
    Join(Table);
} 