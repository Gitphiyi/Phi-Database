# pragma once

#include "general/Types.hpp"
#include "storage-manager/ops/StorageOps.hpp"

namespace DB {
    CondFn equals();

    class Selection : public StorageOps {
        public: 
            Selection(StorageOps* child, CondFn cond);
            void open() override;
            void close() override;
        protected:
            StorageOps*     childOp;
            CondFn          condition;
    };

    struct NaiveSelection : public Selection {
        NaiveSelection(StorageOps* child, CondFn cond)
            : Selection(child, cond) {}
        std::vector<Row*> next() override;
    };

    struct VectorizedSelection : public Selection {
        VectorizedSelection(StorageOps* child, CondFn cond)
            : Selection(child, cond) {}
        std::vector<Row*> next() override;
    };
}
