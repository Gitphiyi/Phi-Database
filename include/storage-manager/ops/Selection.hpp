# pragma once

#include "storage-manager/ops/StorageOps.hpp"

struct NaiveSelection : public Selection {
    NaiveSelection(StorageOps* child, std::function<bool(const Row*)> cond)
        : Selection(child, cond) {}
    std::vector<Row*> next() override;
};

struct VectorizedSelection : public Selection {
    VectorizedSelection(StorageOps* child, std::function<bool(const Row*)> cond)
        : Selection(child, cond) {}
    std::vector<Row*> next() override;
};