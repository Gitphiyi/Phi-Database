#pragma once

#include <unordered_map>
#include <vector>
#include <iostream>
#include <mutex>

#include "general/Types.hpp"
#include "general/Structs.hpp"
#include "scheduling/ScheduleStructs.hpp"

namespace DB {
    class TScheduler {
        public: 
            TScheduler();
            void schedule_transaction(); //scheduling of operations so they don't mess up
            void receive_ops(std::vector<Operation> transaction);
            void clear_scheduler();

        private:
            std::unordered_map<u16, std::vector<Operation*>>        theTransactions;
            
            std::unordered_map<string, std::vector<Operation*>>     order_transaction(std::vector<Operation*>& transaction);
    };
}