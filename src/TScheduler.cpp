#include "TScheduler.hpp"
#include <iostream>
#include <algorithm>

using namespace DB;

TScheduler::TScheduler() {
    std::cout << "initializing Transaction Scheduler\n";
}

std::vector<Operation> TScheduler::order_transactions() {
    std::unordered_map<string, std::vector<Operation>> table_partition{};
    std::vector<Operation> schedule{};
    u16 transaction_w_lock;

    for(auto it = theTransactions.begin(); it != theTransactions.end(); it++) {
        transaction_w_lock = it->first;
        std::vector<Operation>& transaction = it->second;

        //group operations in order of same transaction by which table they modify
        for(Operation& op : transaction) {
            table_partition.try_emplace(op.filename, std::vector<Operation>{});
            table_partition[op.filename].emplace_back(op);
        }
        //flatten the map and add it to the schedule 
        for(auto jt = table_partition.begin(); jt != table_partition.end(); ++jt) {
            for (Operation& top : jt->second) {
                schedule.emplace_back(top);
            }
        }
    }
    return schedule;
}

void TScheduler::receive_ops(std::vector<Operation> transaction) {
    for(int i = 0; i < transaction.size(); i++) {
        Operation& op = transaction[i];
        theTransactions.try_emplace(op.transaction_id, std::vector<Operation>{});
        theTransactions[op.transaction_id].push_back(op);
    }
}

void TScheduler::clear_scheduler() {
    for(auto it = theTransactions.begin(); it != theTransactions.end(); it++) {
        std::vector<Operation>& vec = theTransactions[it->first];
        vec.erase(vec.begin(), vec.end());
    }
}
