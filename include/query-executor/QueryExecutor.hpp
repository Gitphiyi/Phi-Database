#pragma once

#include "sql-compiler/SqlAST.hpp"
#include "sql-compiler/Parser.hpp"
#include "storage-manager/Table.hpp"
#include "storage-manager/HeapFile.hpp"
#include "storage-manager/ops/StorageOps.hpp"
#include "storage-manager/ops/Selection.hpp"

#include <unordered_map>
#include <memory>
#include <functional>
#include <stdexcept>

namespace DB {

struct QueryResult {
    std::vector<string> column_names;
    std::vector<Row*> rows;
    bool success = true;
    string error_message;
    int64_t rows_affected = 0;

    void print() const {
        if (!success) {
            std::cout << "Error: " << error_message << std::endl;
            return;
        }

        for (size_t i = 0; i < column_names.size(); i++) {
            if (i > 0) std::cout << " | ";
            std::cout << column_names[i];
        }
        std::cout << std::endl;

        for (size_t i = 0; i < column_names.size(); i++) {
            if (i > 0) std::cout << "-+-";
            std::cout << "--------";
        }
        std::cout << std::endl;

        for (const auto& row : rows) {
            for (size_t i = 0; i < row->values.size(); i++) {
                if (i > 0) std::cout << " | ";
                std::visit([](auto&& val) {
                    using T = std::decay_t<decltype(val)>;
                    if constexpr (std::is_same_v<T, string>) {
                        std::cout << val;
                    } else if constexpr (std::is_same_v<T, bool>) {
                        std::cout << (val ? "true" : "false");
                    } else {
                        std::cout << val;
                    }
                }, row->values[i]);
            }
            std::cout << std::endl;
        }
        std::cout << "\n(" << rows.size() << " rows)" << std::endl;
    }
};

class Catalog {
public:
    void addTable(const string& name, Table* table) {
        tables_[name] = table;
    }

    Table* getTable(const string& name) {
        auto it = tables_.find(name);
        if (it == tables_.end()) {
            return nullptr;
        }
        return it->second;
    }

    bool hasTable(const string& name) const {
        return tables_.find(name) != tables_.end();
    }

    void removeTable(const string& name) {
        tables_.erase(name);
    }

    const std::unordered_map<string, Table*>& getAllTables() const {
        return tables_;
    }

private:
    std::unordered_map<string, Table*> tables_;
};

class QueryExecutor {
public:
    QueryExecutor(Catalog& catalog);
    QueryResult execute(const string& sql);
    QueryResult executeRA(const RANodePtr& ra_tree);

private:
    Catalog& catalog_;

    StorageOps* buildOperatorTree(const RANodePtr& node);
    QueryResult executeSelect(const RANodePtr& node);
    QueryResult executeInsert(const RANodePtr& node);
    QueryResult executeUpdate(const RANodePtr& node);
    QueryResult executeDelete(const RANodePtr& node);
    QueryResult executeCreateTable(const RANodePtr& node);
    QueryResult executeDropTable(const RANodePtr& node);
    datatype evaluateExpression(const ExprPtr& expr, Row* row, const Schema& schema);
    bool evaluatePredicate(const ExprPtr& pred, Row* row, const Schema& schema);
    int getColumnIndex(const Schema& schema, const string& column_name);
};

class ProjectOp : public StorageOps {
public:
    ProjectOp(StorageOps* child, const std::vector<ExprPtr>& projections,
              const Schema& schema);

    void open() override;
    std::vector<Row*> next() override;
    void close() override;

private:
    StorageOps* child_;
    std::vector<ExprPtr> projections_;
    const Schema& schema_;
};

class FilterOp : public StorageOps {
public:
    FilterOp(StorageOps* child, const ExprPtr& predicate, const Schema& schema);

    void open() override;
    std::vector<Row*> next() override;
    void close() override;

private:
    StorageOps* child_;
    ExprPtr predicate_;
    const Schema& schema_;

    bool evaluatePredicate(Row* row);
    datatype evaluateExpression(const ExprPtr& expr, Row* row);
};

class LimitOp : public StorageOps {
public:
    LimitOp(StorageOps* child, int64_t limit, int64_t offset);

    void open() override;
    std::vector<Row*> next() override;
    void close() override;

private:
    StorageOps* child_;
    int64_t limit_;
    int64_t offset_;
    int64_t current_offset_;
    int64_t rows_returned_;
};

class CrossProductOp : public StorageOps {
public:
    CrossProductOp(StorageOps* left, StorageOps* right);

    void open() override;
    std::vector<Row*> next() override;
    void close() override;

private:
    StorageOps* left_;
    StorageOps* right_;
    std::vector<Row*> left_rows_;
    std::vector<Row*> right_rows_;
    size_t left_idx_;
    size_t right_idx_;
    bool initialized_;
};

class NestedLoopJoin : public StorageOps {
public:
    NestedLoopJoin(StorageOps* left, StorageOps* right,
                   const ExprPtr& condition, RANodeType join_type,
                   const Schema& left_schema, const Schema& right_schema);

    void open() override;
    std::vector<Row*> next() override;
    void close() override;

private:
    StorageOps* left_;
    StorageOps* right_;
    ExprPtr condition_;
    RANodeType join_type_;
    const Schema& left_schema_;
    const Schema& right_schema_;
    std::vector<Row*> left_rows_;
    std::vector<Row*> right_rows_;
    size_t left_idx_;
    size_t right_idx_;
    bool initialized_;

    bool evaluateCondition(Row* left_row, Row* right_row);
    Row* combineRows(Row* left_row, Row* right_row);
};

} // namespace DB
