#include "../include/Database.hpp"

namespace db {
    Database::Database(u64 pgSize, Logger& logger, PageCache& cache) : 
        thePageSize(pgSize),
        theTables(),
        theLogger(logger),
        thePageCache(cache)
    {

    }

    Database::write_header_page() {
        
    }
   
}

