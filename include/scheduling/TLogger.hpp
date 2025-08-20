#pragma once 

#include <deque>
#include <vector>
#include <unordered_set>
#include <iostream>

#include "general/Types.hpp"
#include "general/Structs.hpp"
#include "PageCache.hpp"
#include "table/Table.hpp"
#include "Database.hpp"
#include "scheduling/ScheduleStructs.hpp"

namespace DB {
    class TLogger {
        public:
            TLogger(PageCache& cache, Database& database, TScheduler& scheduler);
            void add_ops(std::vector<Operation>& operations);
            void retrieve_dests(); //get set of destinations that could have transaction issues
            void flush_ops();
            void clear_ops();
            void print();

        private:
            PageCache&                      theCache;
            Database&                       theDatabase;
            TScheduler&                     theScheduler;
            std::unordered_set<string*>     theDestSet;
            std::deque<Operation*>          theOps;
            bool                            theFlushedStatus;

    };
}