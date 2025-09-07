#include "storage-manager/ops/Selection.hpp"

#include <iostream>

namespace DB {
    std::vector<Row*> NaiveSelection::next() {
        std::vector<Row*> result;
        auto batch_rows = childOp->next();
        int i = 0;
        for(Row* r : batch_rows) {
            if(i < 10) {
                result.push_back(r);
            }
            i++;
        }
        return result;
    }
}
