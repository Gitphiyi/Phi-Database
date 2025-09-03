#pragma once

#include "Table.hpp"

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
struct StorageOps {
    virtual ~StorageOps();
    //set to 0 means pure virtual function
    virtual open() = 0; // initializes resources
    virtual next() = 0; // produces block of tuples (size of cacheline and does operation on it)
    virtual close() = 0 ;// closes all the resources
}

struct Selection : StorageOps {

}

struct Join final : StorageOps {
    Join(Table);
} 