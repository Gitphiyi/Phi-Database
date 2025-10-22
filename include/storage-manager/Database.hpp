#pragma once 

#include "general/Types.hpp"
#include "table/Table.hpp"

#include <vector>
#include <stack>

namespace DB {
    struct Database {
        string name;
        Table* tables;
        int num_tables;
    };

    
}