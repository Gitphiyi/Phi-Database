#include "storage-manager/BPlusTree.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>

using namespace DB;

BPlusTree::BPlusTree(int order) : root(nullptr), order(order) {
    maxKeys = order - 1;
    minKeys = (order + 1) / 2 - 1;
}

bool BPlusTree::keyLess(const datatype& a, const datatype& b) const {
    if (a.index() != b.index()) {
        throw std::runtime_error("Cannot compare different types");
    }

    return std::visit([](auto&& lhs, auto&& rhs) -> bool {
        using T = std::decay_t<decltype(lhs)>;
        using U = std::decay_t<decltype(rhs)>;
        if constexpr (std::is_same_v<T, U>) {
            return lhs < rhs;
        }
        return false;
    }, a, b);
}

bool BPlusTree::keyEqual(const datatype& a, const datatype& b) const {
    if (a.index() != b.index()) {
        return false;
    }

    return std::visit([](auto&& lhs, auto&& rhs) -> bool {
        using T = std::decay_t<decltype(lhs)>;
        using U = std::decay_t<decltype(rhs)>;
        if constexpr (std::is_same_v<T, U>) {
            return lhs == rhs;
        }
        return false;
    }, a, b);
}

bool BPlusTree::keyLessOrEqual(const datatype& a, const datatype& b) const {
    return keyLess(a, b) || keyEqual(a, b);
}

int BPlusTree::findKeyIndex(const std::vector<datatype>& keys, const datatype& key) const {
    int idx = 0;
    while (idx < static_cast<int>(keys.size()) && keyLess(keys[idx], key)) {
        idx++;
    }
    return idx;
}

std::shared_ptr<BPlusNode> BPlusTree::findLeaf(const datatype& key) const {
    if (!root) return nullptr;

    auto node = root;
    while (!node->isLeaf) {
        int idx = findKeyIndex(node->keys, key);
        if (idx < static_cast<int>(node->keys.size()) && keyEqual(node->keys[idx], key)) {
            idx++;
        }
        node = node->children[idx];
    }
    return node;
}

void BPlusTree::insert(const datatype& key, const RowId& rid) {
    if (!root) {
        root = std::make_shared<BPlusNode>(true);
        root->keys.push_back(key);
        root->rowIds.push_back(rid);
        return;
    }

    auto leaf = findLeaf(key);
    insertIntoLeaf(leaf, key, rid);

    if (static_cast<int>(leaf->keys.size()) > maxKeys) {
        splitLeaf(leaf);
    }
}

void BPlusTree::insertIntoLeaf(std::shared_ptr<BPlusNode> leaf, const datatype& key, const RowId& rid) {
    int idx = findKeyIndex(leaf->keys, key);
    leaf->keys.insert(leaf->keys.begin() + idx, key);
    leaf->rowIds.insert(leaf->rowIds.begin() + idx, rid);
}

void BPlusTree::splitLeaf(std::shared_ptr<BPlusNode> leaf) {
    auto newLeaf = std::make_shared<BPlusNode>(true);
    int mid = (leaf->keys.size() + 1) / 2;

    newLeaf->keys.assign(leaf->keys.begin() + mid, leaf->keys.end());
    newLeaf->rowIds.assign(leaf->rowIds.begin() + mid, leaf->rowIds.end());

    leaf->keys.resize(mid);
    leaf->rowIds.resize(mid);

    newLeaf->next = leaf->next;
    leaf->next = newLeaf;

    datatype splitKey = newLeaf->keys[0];
    insertIntoParent(leaf, splitKey, newLeaf);
}

void BPlusTree::insertIntoParent(std::shared_ptr<BPlusNode> left, const datatype& key, std::shared_ptr<BPlusNode> right) {
    auto parent = left->parent.lock();

    if (!parent) {
        auto newRoot = std::make_shared<BPlusNode>(false);
        newRoot->keys.push_back(key);
        newRoot->children.push_back(left);
        newRoot->children.push_back(right);
        left->parent = newRoot;
        right->parent = newRoot;
        root = newRoot;
        return;
    }

    int idx = findKeyIndex(parent->keys, key);
    parent->keys.insert(parent->keys.begin() + idx, key);
    parent->children.insert(parent->children.begin() + idx + 1, right);
    right->parent = parent;

    if (static_cast<int>(parent->keys.size()) > maxKeys) {
        splitInternal(parent);
    }
}

void BPlusTree::splitInternal(std::shared_ptr<BPlusNode> node) {
    auto newNode = std::make_shared<BPlusNode>(false);
    int mid = node->keys.size() / 2;

    datatype promoteKey = node->keys[mid];

    newNode->keys.assign(node->keys.begin() + mid + 1, node->keys.end());
    newNode->children.assign(node->children.begin() + mid + 1, node->children.end());

    for (auto& child : newNode->children) {
        child->parent = newNode;
    }

    node->keys.resize(mid);
    node->children.resize(mid + 1);

    insertIntoParent(node, promoteKey, newNode);
}

std::optional<RowId> BPlusTree::search(const datatype& key) const {
    auto leaf = findLeaf(key);
    if (!leaf) return std::nullopt;

    for (size_t i = 0; i < leaf->keys.size(); i++) {
        if (keyEqual(leaf->keys[i], key)) {
            return leaf->rowIds[i];
        }
    }
    return std::nullopt;
}

std::vector<RowId> BPlusTree::searchAll(const datatype& key) const {
    std::vector<RowId> results;
    auto leaf = findLeaf(key);
    if (!leaf) return results;

    for (size_t i = 0; i < leaf->keys.size(); i++) {
        if (keyEqual(leaf->keys[i], key)) {
            results.push_back(leaf->rowIds[i]);
        }
    }

    auto next = leaf->next;
    while (next && !next->keys.empty() && keyEqual(next->keys[0], key)) {
        for (size_t i = 0; i < next->keys.size(); i++) {
            if (keyEqual(next->keys[i], key)) {
                results.push_back(next->rowIds[i]);
            } else {
                break;
            }
        }
        next = next->next;
    }

    return results;
}

std::vector<RowId> BPlusTree::rangeSearch(const datatype& low, const datatype& high) const {
    std::vector<RowId> results;
    auto leaf = findLeaf(low);
    if (!leaf) return results;

    bool started = false;
    while (leaf) {
        for (size_t i = 0; i < leaf->keys.size(); i++) {
            if (!started && keyLess(leaf->keys[i], low)) {
                continue;
            }
            started = true;

            if (keyLessOrEqual(leaf->keys[i], high)) {
                results.push_back(leaf->rowIds[i]);
            } else {
                return results;
            }
        }
        leaf = leaf->next;
    }

    return results;
}

bool BPlusTree::remove(const datatype& key) {
    auto leaf = findLeaf(key);
    if (!leaf) return false;

    int idx = -1;
    for (size_t i = 0; i < leaf->keys.size(); i++) {
        if (keyEqual(leaf->keys[i], key)) {
            idx = i;
            break;
        }
    }

    if (idx == -1) return false;

    leaf->keys.erase(leaf->keys.begin() + idx);
    leaf->rowIds.erase(leaf->rowIds.begin() + idx);

    if (leaf == root && leaf->keys.empty()) {
        root = nullptr;
    }

    return true;
}

void BPlusTree::clear() {
    root = nullptr;
}

void BPlusTree::print() const {
    if (!root) {
        std::cout << "Empty tree" << std::endl;
        return;
    }
    printNode(root, 0);
}

void BPlusTree::printNode(std::shared_ptr<BPlusNode> node, int level) const {
    std::string indent(level * 2, ' ');
    std::cout << indent << (node->isLeaf ? "Leaf" : "Internal") << ": [";

    for (size_t i = 0; i < node->keys.size(); i++) {
        std::visit([](auto&& val) { std::cout << val; }, node->keys[i]);
        if (i < node->keys.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;

    if (!node->isLeaf) {
        for (auto& child : node->children) {
            printNode(child, level + 1);
        }
    }
}

Index::Index(const string& name, const string& table, int colIdx)
    : indexName(name), tableName(table), columnIdx(colIdx), tree(BTREE_ORDER) {}

void Index::insert(const datatype& key, const RowId& rid) {
    tree.insert(key, rid);
}

bool Index::remove(const datatype& key) {
    return tree.remove(key);
}

std::optional<RowId> Index::lookup(const datatype& key) const {
    return tree.search(key);
}

std::vector<RowId> Index::lookupAll(const datatype& key) const {
    return tree.searchAll(key);
}

std::vector<RowId> Index::rangeLookup(const datatype& low, const datatype& high) const {
    return tree.rangeSearch(low, high);
}
