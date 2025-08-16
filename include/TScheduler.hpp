#pragma once

#include "Table.hpp"
#include <vector>
#include <iostream>
#include <mutex>

namespace DB {
    class TScheduler {
        public: 
            TScheduler(Table& table, std::vector<Operation> ops);


        private:
            Table&                  theTable;
            std::vector<Operation>  theOperations;
            std::mutex              theMutex;
            
    };
}