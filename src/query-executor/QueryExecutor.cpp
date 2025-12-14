#include "query-executor/QueryExecutor.hpp"
#include "sql-compiler/Lexer.hpp"

#include <iostream>
#include <algorithm>

namespace DB {

QueryExecutor::QueryExecutor(Catalog& catalog) : catalog_(catalog) {}

QueryResult QueryExecutor::execute(const string& sql) {
    try {
        string query = sql;
        RANodePtr ra_tree = compile_sql(query);
        if (!ra_tree) {
            QueryResult result;
            result.success = false;
            result.error_message = "Failed to parse SQL query";
            return result;
        }
        return executeRA(ra_tree);
    } catch (const std::exception& e) {
        QueryResult result;
        result.success = false;
        result.error_message = e.what();
        return result;
    }
}

QueryResult QueryExecutor::executeRA(const RANodePtr& ra_tree) {
    if (!ra_tree) {
        QueryResult result;
        result.success = false;
        result.error_message = "Null RA tree";
        return result;
    }

    switch (ra_tree->type) {
        case RANodeType::PROJECT:
        case RANodeType::SELECT_OP:
        case RANodeType::TABLE_SCAN:
        case RANodeType::INNER_JOIN:
        case RANodeType::LEFT_JOIN:
        case RANodeType::RIGHT_JOIN:
        case RANodeType::FULL_JOIN:
        case RANodeType::CROSS_PRODUCT:
        case RANodeType::LIMIT_OP:
        case RANodeType::SORT:
        case RANodeType::DISTINCT:
        case RANodeType::GROUP_BY:
            return executeSelect(ra_tree);

        case RANodeType::INSERT_OP:
            return executeInsert(ra_tree);

        case RANodeType::UPDATE_OP:
            return executeUpdate(ra_tree);

        case RANodeType::DELETE_OP:
            return executeDelete(ra_tree);

        case RANodeType::CREATE_TABLE_OP:
            return executeCreateTable(ra_tree);

        case RANodeType::DROP_TABLE_OP:
            return executeDropTable(ra_tree);

        default:
            QueryResult result;
            result.success = false;
            result.error_message = "Unknown operation type";
            return result;
    }
}

StorageOps* QueryExecutor::buildOperatorTree(const RANodePtr& node) {
    if (!node) return nullptr;

    switch (node->type) {
        case RANodeType::TABLE_SCAN: {
            Table* table = catalog_.getTable(node->table_name);
            if (!table) {
                throw std::runtime_error("Table not found: " + node->table_name);
            }
            return new SeqScan(*table, 64);
        }

        case RANodeType::SELECT_OP: {
            StorageOps* child = buildOperatorTree(node->left);
            RANodePtr current = node->left;
            while (current && current->type != RANodeType::TABLE_SCAN) {
                current = current->left;
            }
            if (!current) {
                throw std::runtime_error("Could not find base table for selection");
            }
            Table* table = catalog_.getTable(current->table_name);
            if (!table) {
                throw std::runtime_error("Table not found: " + current->table_name);
            }
            return new FilterOp(child, node->predicate, table->getSchema());
        }

        case RANodeType::LIMIT_OP: {
            StorageOps* child = buildOperatorTree(node->left);
            return new LimitOp(child, node->limit_count, node->offset_count);
        }

        case RANodeType::CROSS_PRODUCT: {
            StorageOps* left = buildOperatorTree(node->left);
            StorageOps* right = buildOperatorTree(node->right);
            return new CrossProductOp(left, right);
        }

        case RANodeType::PROJECT: {
            StorageOps* child = buildOperatorTree(node->left);
            RANodePtr current = node->left;
            while (current && current->type != RANodeType::TABLE_SCAN) {
                current = current->left;
            }
            if (!current) {
                throw std::runtime_error("Could not find base table for projection");
            }
            Table* table = catalog_.getTable(current->table_name);
            if (!table) {
                throw std::runtime_error("Table not found: " + current->table_name);
            }
            return new ProjectOp(child, node->projections, table->getSchema());
        }

        default:
            throw std::runtime_error("Unsupported operator type: " + ra_type_to_string(node->type));
    }
}

QueryResult QueryExecutor::executeSelect(const RANodePtr& node) {
    QueryResult result;

    try {
        StorageOps* ops = buildOperatorTree(node);
        ops->open();

        std::vector<Row*> batch;
        while (!(batch = ops->next()).empty()) {
            for (Row* row : batch) {
                result.rows.push_back(row);
            }
        }

        ops->close();

        RANodePtr proj_node = node;
        while (proj_node && proj_node->type != RANodeType::PROJECT) {
            proj_node = proj_node->left;
        }
        if (proj_node && !proj_node->select_all) {
            for (size_t i = 0; i < proj_node->projections.size(); i++) {
                if (i < proj_node->projection_aliases.size() &&
                    !proj_node->projection_aliases[i].empty()) {
                    result.column_names.push_back(proj_node->projection_aliases[i]);
                } else {
                    result.column_names.push_back(expr_to_string(proj_node->projections[i]));
                }
            }
        }

        result.success = true;
        delete ops;
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = e.what();
    }

    return result;
}

QueryResult QueryExecutor::executeInsert(const RANodePtr& node) {
    QueryResult result;

    try {
        Table* table = catalog_.getTable(node->table_name);
        if (!table) {
            result.success = false;
            result.error_message = "Table not found: " + node->table_name;
            return result;
        }

        for (const auto& value_row : node->insert_values) {
            std::vector<datatype> values;
            for (const auto& expr : value_row) {
                switch (expr->type) {
                    case ExprType::LITERAL_INT:
                        values.push_back(static_cast<int>(std::get<int64_t>(expr->literal_value)));
                        break;
                    case ExprType::LITERAL_FLOAT:
                        values.push_back(static_cast<float>(std::get<double>(expr->literal_value)));
                        break;
                    case ExprType::LITERAL_STRING:
                        values.push_back(std::get<string>(expr->literal_value));
                        break;
                    case ExprType::LITERAL_BOOL:
                        values.push_back(std::get<bool>(expr->literal_value));
                        break;
                    default:
                        values.push_back(0);
                        break;
                }
            }

            Row* row = create_row(static_cast<int>(values.size()), std::move(values));
            table->insert_row();
            result.rows_affected++;
            delete row;
        }

        result.success = true;
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = e.what();
    }

    return result;
}

QueryResult QueryExecutor::executeUpdate(const RANodePtr& node) {
    QueryResult result;
    result.success = false;
    result.error_message = "UPDATE not yet implemented";
    return result;
}

QueryResult QueryExecutor::executeDelete(const RANodePtr& node) {
    QueryResult result;
    result.success = false;
    result.error_message = "DELETE not yet implemented";
    return result;
}

QueryResult QueryExecutor::executeCreateTable(const RANodePtr& node) {
    QueryResult result;

    try {
        if (catalog_.hasTable(node->table_name)) {
            result.success = false;
            result.error_message = "Table already exists: " + node->table_name;
            return result;
        }

        Schema schema(node->column_defs.size());
        for (const auto& col_def : node->column_defs) {
            ColumnType col_type = ColumnType::INT;
            if (col_def.type_name == "INT" || col_def.type_name == "INTEGER") {
                col_type = ColumnType::INT;
            } else if (col_def.type_name == "VARCHAR" || col_def.type_name == "TEXT" ||
                       col_def.type_name == "STRING") {
                col_type = ColumnType::STRING;
            } else if (col_def.type_name == "FLOAT" || col_def.type_name == "REAL" ||
                       col_def.type_name == "DOUBLE") {
                col_type = ColumnType::DOUBLE;
            } else if (col_def.type_name == "BOOL" || col_def.type_name == "BOOLEAN") {
                col_type = ColumnType::BOOL;
            }

            schema.add_col(col_def.name, col_type, col_def.nullable,
                          col_def.primary_key, false);
        }

        HeapFile* heapfile = create_heapfile(node->table_name);
        Table* table = new Table(node->table_name, schema, *heapfile);
        catalog_.addTable(node->table_name, table);

        result.success = true;
        result.rows_affected = 0;
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = e.what();
    }

    return result;
}

QueryResult QueryExecutor::executeDropTable(const RANodePtr& node) {
    QueryResult result;

    try {
        if (!catalog_.hasTable(node->table_name)) {
            result.success = false;
            result.error_message = "Table does not exist: " + node->table_name;
            return result;
        }

        catalog_.removeTable(node->table_name);
        result.success = true;
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = e.what();
    }

    return result;
}

int QueryExecutor::getColumnIndex(const Schema& schema, const string& column_name) {
    for (size_t i = 0; i < schema.columns.size(); i++) {
        if (schema.columns[i].name == column_name) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

FilterOp::FilterOp(StorageOps* child, const ExprPtr& predicate, const Schema& schema)
    : child_(child), predicate_(predicate), schema_(schema) {}

void FilterOp::open() {
    child_->open();
}

std::vector<Row*> FilterOp::next() {
    std::vector<Row*> result;
    std::vector<Row*> batch = child_->next();

    for (Row* row : batch) {
        if (evaluatePredicate(row)) {
            result.push_back(row);
        }
    }

    return result;
}

void FilterOp::close() {
    child_->close();
}

bool FilterOp::evaluatePredicate(Row* row) {
    if (!predicate_) return true;

    datatype result = evaluateExpression(predicate_, row);
    if (std::holds_alternative<bool>(result)) {
        return std::get<bool>(result);
    }
    return false;
}

datatype FilterOp::evaluateExpression(const ExprPtr& expr, Row* row) {
    if (!expr) return false;

    switch (expr->type) {
        case ExprType::LITERAL_INT:
            return static_cast<int>(std::get<int64_t>(expr->literal_value));
        case ExprType::LITERAL_FLOAT:
            return static_cast<float>(std::get<double>(expr->literal_value));
        case ExprType::LITERAL_STRING:
            return std::get<string>(expr->literal_value);
        case ExprType::LITERAL_BOOL:
            return std::get<bool>(expr->literal_value);
        case ExprType::LITERAL_NULL:
            return 0;

        case ExprType::COLUMN_REF: {
            for (size_t i = 0; i < schema_.columns.size(); i++) {
                if (schema_.columns[i].name == expr->column_name) {
                    if (i < row->values.size()) {
                        return row->values[i];
                    }
                }
            }
            return 0;
        }

        case ExprType::BINARY_OP: {
            datatype left_val = evaluateExpression(expr->children[0], row);
            datatype right_val = evaluateExpression(expr->children[1], row);

            switch (expr->binary_op) {
                case BinaryOp::EQ:
                    return left_val == right_val;
                case BinaryOp::NE:
                    return left_val != right_val;
                case BinaryOp::LT:
                    return left_val < right_val;
                case BinaryOp::LE:
                    return left_val <= right_val;
                case BinaryOp::GT:
                    return left_val > right_val;
                case BinaryOp::GE:
                    return left_val >= right_val;
                case BinaryOp::AND: {
                    bool l = std::holds_alternative<bool>(left_val) ?
                             std::get<bool>(left_val) : false;
                    bool r = std::holds_alternative<bool>(right_val) ?
                             std::get<bool>(right_val) : false;
                    return l && r;
                }
                case BinaryOp::OR: {
                    bool l = std::holds_alternative<bool>(left_val) ?
                             std::get<bool>(left_val) : false;
                    bool r = std::holds_alternative<bool>(right_val) ?
                             std::get<bool>(right_val) : false;
                    return l || r;
                }
                default:
                    return false;
            }
        }

        case ExprType::UNARY_OP: {
            datatype val = evaluateExpression(expr->children[0], row);
            switch (expr->unary_op) {
                case UnaryOp::NOT: {
                    bool b = std::holds_alternative<bool>(val) ?
                             std::get<bool>(val) : false;
                    return !b;
                }
                default:
                    return val;
            }
        }

        default:
            return false;
    }
}

ProjectOp::ProjectOp(StorageOps* child, const std::vector<ExprPtr>& projections,
                     const Schema& schema)
    : child_(child), projections_(projections), schema_(schema) {}

void ProjectOp::open() {
    child_->open();
}

std::vector<Row*> ProjectOp::next() {
    std::vector<Row*> input = child_->next();
    if (input.empty()) return input;

    std::vector<Row*> result;
    for (Row* row : input) {
        std::vector<datatype> new_values;

        for (const auto& proj : projections_) {
            if (proj->type == ExprType::COLUMN_REF) {
                for (size_t i = 0; i < schema_.columns.size(); i++) {
                    if (schema_.columns[i].name == proj->column_name) {
                        if (i < row->values.size()) {
                            new_values.push_back(row->values[i]);
                        }
                        break;
                    }
                }
            }
        }

        Row* new_row = create_row(static_cast<int>(new_values.size()), std::move(new_values));
        result.push_back(new_row);
    }

    return result;
}

void ProjectOp::close() {
    child_->close();
}

LimitOp::LimitOp(StorageOps* child, int64_t limit, int64_t offset)
    : child_(child), limit_(limit), offset_(offset),
      current_offset_(0), rows_returned_(0) {}

void LimitOp::open() {
    child_->open();
    current_offset_ = 0;
    rows_returned_ = 0;
}

std::vector<Row*> LimitOp::next() {
    if (limit_ >= 0 && rows_returned_ >= limit_) {
        return {};
    }

    std::vector<Row*> result;
    std::vector<Row*> batch = child_->next();

    for (Row* row : batch) {
        if (current_offset_ < offset_) {
            current_offset_++;
            continue;
        }

        if (limit_ >= 0 && rows_returned_ >= limit_) {
            break;
        }

        result.push_back(row);
        rows_returned_++;
    }

    return result;
}

void LimitOp::close() {
    child_->close();
}

CrossProductOp::CrossProductOp(StorageOps* left, StorageOps* right)
    : left_(left), right_(right), left_idx_(0), right_idx_(0), initialized_(false) {}

void CrossProductOp::open() {
    left_->open();
    right_->open();
    initialized_ = false;
    left_idx_ = 0;
    right_idx_ = 0;
}

std::vector<Row*> CrossProductOp::next() {
    if (!initialized_) {
        std::vector<Row*> batch;
        while (!(batch = left_->next()).empty()) {
            for (Row* row : batch) {
                left_rows_.push_back(row);
            }
        }
        while (!(batch = right_->next()).empty()) {
            for (Row* row : batch) {
                right_rows_.push_back(row);
            }
        }
        initialized_ = true;
    }

    std::vector<Row*> result;
    const size_t BATCH_SIZE = 64;

    while (result.size() < BATCH_SIZE && left_idx_ < left_rows_.size()) {
        Row* left_row = left_rows_[left_idx_];
        Row* right_row = right_rows_[right_idx_];

        std::vector<datatype> combined_values;
        for (const auto& val : left_row->values) {
            combined_values.push_back(val);
        }
        for (const auto& val : right_row->values) {
            combined_values.push_back(val);
        }

        Row* combined = create_row(static_cast<int>(combined_values.size()),
                                   std::move(combined_values));
        result.push_back(combined);

        right_idx_++;
        if (right_idx_ >= right_rows_.size()) {
            right_idx_ = 0;
            left_idx_++;
        }
    }

    return result;
}

void CrossProductOp::close() {
    left_->close();
    right_->close();
}

NestedLoopJoin::NestedLoopJoin(StorageOps* left, StorageOps* right,
                               const ExprPtr& condition, RANodeType join_type,
                               const Schema& left_schema, const Schema& right_schema)
    : left_(left), right_(right), condition_(condition), join_type_(join_type),
      left_schema_(left_schema), right_schema_(right_schema),
      left_idx_(0), right_idx_(0), initialized_(false) {}

void NestedLoopJoin::open() {
    left_->open();
    right_->open();
    initialized_ = false;
    left_idx_ = 0;
    right_idx_ = 0;
}

std::vector<Row*> NestedLoopJoin::next() {
    if (!initialized_) {
        std::vector<Row*> batch;
        while (!(batch = left_->next()).empty()) {
            for (Row* row : batch) {
                left_rows_.push_back(row);
            }
        }
        while (!(batch = right_->next()).empty()) {
            for (Row* row : batch) {
                right_rows_.push_back(row);
            }
        }
        initialized_ = true;
    }

    std::vector<Row*> result;
    const size_t BATCH_SIZE = 64;

    while (result.size() < BATCH_SIZE && left_idx_ < left_rows_.size()) {
        Row* left_row = left_rows_[left_idx_];
        Row* right_row = right_rows_[right_idx_];

        if (evaluateCondition(left_row, right_row)) {
            result.push_back(combineRows(left_row, right_row));
        }

        right_idx_++;
        if (right_idx_ >= right_rows_.size()) {
            right_idx_ = 0;
            left_idx_++;
        }
    }

    return result;
}

void NestedLoopJoin::close() {
    left_->close();
    right_->close();
}

bool NestedLoopJoin::evaluateCondition(Row* left_row, Row* right_row) {
    if (!condition_) return true;
    return true;
}

Row* NestedLoopJoin::combineRows(Row* left_row, Row* right_row) {
    std::vector<datatype> combined_values;
    for (const auto& val : left_row->values) {
        combined_values.push_back(val);
    }
    for (const auto& val : right_row->values) {
        combined_values.push_back(val);
    }
    return create_row(static_cast<int>(combined_values.size()), std::move(combined_values));
}

IndexScan::IndexScan(Table& table, int columnIndex, const datatype& key, size_t batch_size)
    : table_(table), columnIndex_(columnIndex), key_(key),
      lowKey_(0), highKey_(0), isRangeScan_(false),
      batchSize_(batch_size), currentIdx_(0) {}

IndexScan::IndexScan(Table& table, int columnIndex, const datatype& low, const datatype& high, size_t batch_size)
    : table_(table), columnIndex_(columnIndex), key_(0),
      lowKey_(low), highKey_(high), isRangeScan_(true),
      batchSize_(batch_size), currentIdx_(0) {}

void IndexScan::open() {
    currentIdx_ = 0;
    if (isRangeScan_) {
        matchingRowIds_ = table_.indexRangeLookup(columnIndex_, lowKey_, highKey_);
    } else {
        matchingRowIds_ = table_.indexLookup(columnIndex_, key_);
    }
}

std::vector<Row*> IndexScan::next() {
    std::vector<Row*> result;

    while (result.size() < batchSize_ && currentIdx_ < matchingRowIds_.size()) {
        Row* row = table_.read_row(matchingRowIds_[currentIdx_]);
        if (row) {
            result.push_back(row);
        }
        currentIdx_++;
    }

    return result;
}

void IndexScan::close() {
    matchingRowIds_.clear();
    currentIdx_ = 0;
}

} // namespace DB
