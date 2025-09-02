#include "TScheduler.hpp"
#include <iostream>
#include <queue>
#include <algorithm>

using namespace DB;

TScheduler::TScheduler(TLogger tlogger) : theTLogger(tlogger) {
    std::cout << "initializing Transaction Scheduler\n";
}

void TScheduler::schedule_transaction() {
    std::vector<Operation*> schedule{};

    for(auto it = theTransactions.begin(); it != theTransactions.end(); ++it) {
        std::queue<Operation*> lockOrder{};
        std::vector<Operation*> transaction = it->second;
        u32 transaction_id = it->first;
        Operation op = *transaction[0];
        std::unordered_map<string, std::vector<Operation*>> table_partition = order_transaction(transaction);

        //add lock ops
        for(auto et = table_partition.begin(); et != table_partition.end(); ++et) {
            std::vector<Operation*> trans = et->second;
            Operation* lock = new Operation(trans[0]->transaction_id, LOCK, trans[0]->filename, nullptr, nullptr);
            lockOrder.push(lock);
            schedule.push_back(lock);
            for(Operation* op_ptr : trans) {
                schedule.push_back(op_ptr);
            }
        }

        //add unlock ops
        while(!lockOrder.empty()) {
            Operation* corresponding_lock = lockOrder.front();
            Operation* unlock = new Operation(corresponding_lock->transaction_id, UNLOCK, corresponding_lock->filename, nullptr, nullptr);
            schedule.push_back(unlock);
            lockOrder.pop();
        }

        //add commit op
        Operation* commit = new Operation(transaction_id, COMMIT, op.filename, nullptr, nullptr);
        schedule.push_back(commit);
    }
            //send over operations to be logged
        //theTLogger.add_ops(schedule);
}

std::unordered_map<string, std::vector<Operation*>> order_transaction(std::vector<Operation*>& transaction) {
    std::unordered_map<string, std::vector<Operation*>> table_partition{};

    //group operations in order of same transaction by which table they modify
    for(Operation* op : transaction) {
        table_partition.try_emplace(op->filename, std::vector<Operation*>{});
        table_partition[op->filename].emplace_back(op);
    }
    return table_partition;
}


void TScheduler::receive_ops(std::vector<Operation> transaction) {
    for(int i = 0; i < transaction.size(); i++) {
        Operation& op = transaction[i];
        theTransactions.try_emplace(op.transaction_id, std::vector<Operation>{});
        theTransactions[op.transaction_id].push_back(&op);
    }
}

void TScheduler::clear_scheduler() {
    for(auto it = theTransactions.begin(); it != theTransactions.end(); it++) {
        std::vector<Operation*>& vec = theTransactions[it->first];
        vec.erase(vec.begin(), vec.end());
    }
}
