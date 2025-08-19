#pragma once

#include <unordered_map>
#include <vector>
#include <iostream>
#include <mutex>

#include "TLogger.hpp"
#include "general/Types.hpp"
#include "general/Structs.hpp"

namespace DB {
    class TScheduler {
        public: 
            TScheduler(TLogger tlogger);
            void schedule_transaction(); //scheduling of operations so they don't mess up
            void receive_ops(std::vector<Operation> transaction);
            void clear_scheduler();

        private:
            TLogger                                                 theTLogger;
            std::unordered_map<u16, std::vector<Operation*>>        theTransactions;
            
            std::unordered_map<string, std::vector<Operation*>>     order_transaction(std::vector<Operation*>& transaction);
    };
}