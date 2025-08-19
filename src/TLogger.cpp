#include "TLogger.hpp"

using namespace DB; 

TLogger::TLogger(PageCache& cache, Database& database, TScheduler& scheduler) :
    theCache(cache),
    theDatabase(database),
    theScheduler(scheduler),
    theFlushedStatus(false)
{

}

void TLogger::retrieve_dests() {

}

void TLogger::add_ops(std::vector<Operation> ops) {
   // std::vector<Operation> ops = theScheduler.schedule_transaction();
   for(Operation op : ops) {
    theOps.emplace_front(op);
   }
}

void TLogger::flush_ops() {
    if(theOps.size() == 0) {
        return;
    }
    u32 curr_transaction = (*theOps.begin())->transaction_id;
    string curr_dest = (*theOps.begin())->filename;
    

    for(auto it = theOps.begin(); it != theOps.end() ; ++it) {
        if(curr_transaction != (*it)->transaction_id) {
            curr_transaction = (*it)->transaction_id;
        }
        if(curr_dest != (*it)->filename) {
            curr_dest = (*it)->filename;
        }

        //calls operation
        (theCache.*((*it)->file_op))((*it)->filename, (*it)->buffer);
    }
    theFlushedStatus = true;
}

void TLogger::clear_ops() {
    theOps.clear();
}