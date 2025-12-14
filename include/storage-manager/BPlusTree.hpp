#pragma once

#include "general/Types.hpp"
#include "storage-manager/StorageStructs.hpp"
#include <vector>
#include <memory>
#include <optional>

namespace DB {

constexpr int BTREE_ORDER = 4;

struct BPlusNode {
    bool isLeaf;
    std::vector<datatype> keys;
    std::vector<std::shared_ptr<BPlusNode>> children;
    std::vector<RowId> rowIds;
    std::shared_ptr<BPlusNode> next;
    std::weak_ptr<BPlusNode> parent;

    BPlusNode(bool leaf = false) : isLeaf(leaf), next(nullptr) {}
};

class BPlusTree {
public:
    BPlusTree(int order = BTREE_ORDER);

    void insert(const datatype& key, const RowId& rid);
    bool remove(const datatype& key);
    std::optional<RowId> search(const datatype& key) const;
    std::vector<RowId> rangeSearch(const datatype& low, const datatype& high) const;
    std::vector<RowId> searchAll(const datatype& key) const;

    bool isEmpty() const { return root == nullptr; }
    void clear();
    void print() const;

private:
    std::shared_ptr<BPlusNode> root;
    int order;
    int minKeys;
    int maxKeys;

    std::shared_ptr<BPlusNode> findLeaf(const datatype& key) const;
    void insertIntoLeaf(std::shared_ptr<BPlusNode> leaf, const datatype& key, const RowId& rid);
    void insertIntoParent(std::shared_ptr<BPlusNode> left, const datatype& key, std::shared_ptr<BPlusNode> right);
    void splitLeaf(std::shared_ptr<BPlusNode> leaf);
    void splitInternal(std::shared_ptr<BPlusNode> node);

    int findKeyIndex(const std::vector<datatype>& keys, const datatype& key) const;
    bool keyLess(const datatype& a, const datatype& b) const;
    bool keyEqual(const datatype& a, const datatype& b) const;
    bool keyLessOrEqual(const datatype& a, const datatype& b) const;

    void printNode(std::shared_ptr<BPlusNode> node, int level) const;
};

class Index {
public:
    Index(const string& name, const string& tableName, int columnIndex);

    void insert(const datatype& key, const RowId& rid);
    bool remove(const datatype& key);
    std::optional<RowId> lookup(const datatype& key) const;
    std::vector<RowId> lookupAll(const datatype& key) const;
    std::vector<RowId> rangeLookup(const datatype& low, const datatype& high) const;

    const string& getName() const { return indexName; }
    const string& getTableName() const { return tableName; }
    int getColumnIndex() const { return columnIdx; }

private:
    string indexName;
    string tableName;
    int columnIdx;
    BPlusTree tree;
};

}
