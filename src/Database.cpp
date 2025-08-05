#include "../include/Database.hpp"

Database::Database(u64 pgSize, Logger& logger, PageCache& cache) : 
    thePageSize(pgSize),
    theTables(),
    theLogger(logger),
    thePageCache(cache)
{

}