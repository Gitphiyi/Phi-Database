#include "TLogger.hpp"

using namespace DB; 

TLogger::TLogger(Table& table, PageCache& cache, Database& database, TScheduler& scheduler) :
    theTable(table),
    theCache(cache),
    theDatabase(database),
    theScheduler(scheduler),
    theFlushedStatus(false)
{}

void TLogger::add_ops(std::vector<Operation> ops) {
    for(size_t i = 0; i < ops.size(); i++){
        theOps.push_back(ops[i]);
    }
}

void TLogger::flush_ops() {
    for(auto it = theOps.begin(); it != theOps.end() ; ++it) {
        //calls operation
        (theCache.*(it->file_op))(it->filename, it->buffer);
    }
    theFlushedStatus = true;
}

void TLogger::clear_ops() {
    theOps.clear();
}