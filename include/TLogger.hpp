#pragma once 

#include <deque>
#include <vector>
#include <iostream>

#include "general/Types.hpp"
#include "general/Structs.hpp"
#include "PageCache.hpp"
#include "table/Table.hpp"
#include "Database.hpp"
#include "TScheduler.hpp"

namespace DB {
    class TLogger {
        public:
            TLogger(Table& table, PageCache& cache, Database& database, TScheduler& scheduler);
            void add_ops(std::vector<Operation> operations);
            void flush_ops();
            void clear_ops();
            void print();

        private:
            PageCache&              theCache;
            Table&                  theTable;
            Database&               theDatabase;
            TScheduler&             theScheduler;
            std::deque<Operation>   theOps;
            bool                    theFlushedStatus;

    };
}