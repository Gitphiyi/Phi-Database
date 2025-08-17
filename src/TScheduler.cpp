#include "TScheduler.hpp"
#include "Structs.hpp"
#include <iostream>

using namespace DB;

TScheduler::TScheduler(TLogger& tLogger) : theTLogger(tLogger) {
    std::cout << "initializing Transaction Scheduler\n";
}

void TScheduler::receive_transaction(std::vector<Operation> transaction) {
    for(int i = 0; i < transaction.size(); i++) {
        theOperations.push_back(transaction[i]);
    }
}

void TScheduler::send_transaction() {

}
