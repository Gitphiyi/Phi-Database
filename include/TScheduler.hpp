#pragma once

#include <vector>
#include <iostream>
#include <mutex>

#include "TLogger.hpp"
#include "Types.hpp"
#include "Structs.hpp"

namespace DB {
    class TScheduler {
        public: 
            TScheduler(TLogger& tLogger);
            void add_op(Operation op);
            void schedule_transactions(); //scheduling of operations so they don't mess up
            void receive_transaction(std::vector<Operation> transaction);
            void send_transaction();
            void pop_op(Operation op);

        private:
            TLogger&                theTLogger;
            std::vector<Operation>  theOperations;
    };
}