#pragma once 

#include "Types.hpp"
#include "PageCache.hpp"
#include "Table.hpp"
#include "Database.hpp"

namespace DB {
    class TLogger {
        public:
            TLogger(const Table& table, const PageCache& cache, const Database& database);

        private:
            PageCache&  theCache;
            Table&      theTable;
            bool        theFlushedStatus;

    };
}