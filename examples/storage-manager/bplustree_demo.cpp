#include "storage-manager/BPlusTree.hpp"
#include <iostream>

using namespace DB;

int main() {
    std::cout << "=== B+ Tree Demo ===" << std::endl;

    BPlusTree tree(4);

    std::cout << "\nInserting keys 1-10..." << std::endl;
    for (int i = 1; i <= 10; i++) {
        RowId rid;
        rid.pageId.heapId = 0;
        rid.pageId.page_num = static_cast<u64>(i);
        rid.record_num = static_cast<u64>(i * 10);
        tree.insert(i, rid);
    }

    std::cout << "\nTree structure:" << std::endl;
    tree.print();

    std::cout << "\nSearching for key 5..." << std::endl;
    auto result = tree.search(5);
    if (result.has_value()) {
        std::cout << "Found: page=" << result->pageId.page_num
                  << ", record=" << result->record_num << std::endl;
    } else {
        std::cout << "Not found" << std::endl;
    }

    std::cout << "\nSearching for key 99 (should not exist)..." << std::endl;
    result = tree.search(99);
    if (result.has_value()) {
        std::cout << "Found: page=" << result->pageId.page_num << std::endl;
    } else {
        std::cout << "Not found (correct)" << std::endl;
    }

    std::cout << "\nRange search [3, 7]..." << std::endl;
    auto range = tree.rangeSearch(3, 7);
    std::cout << "Found " << range.size() << " results:" << std::endl;
    for (const auto& rid : range) {
        std::cout << "  page=" << rid.pageId.page_num
                  << ", record=" << rid.record_num << std::endl;
    }

    std::cout << "\nDeleting key 5..." << std::endl;
    bool deleted = tree.remove(5);
    std::cout << "Deleted: " << (deleted ? "yes" : "no") << std::endl;

    std::cout << "\nSearching for key 5 after deletion..." << std::endl;
    result = tree.search(5);
    if (result.has_value()) {
        std::cout << "Found (unexpected)" << std::endl;
    } else {
        std::cout << "Not found (correct)" << std::endl;
    }

    std::cout << "\n=== Index Class Demo ===" << std::endl;
    Index idx("idx_employee_id", "employees", 0);

    std::cout << "\nInserting employee IDs..." << std::endl;
    for (int i = 100; i < 110; i++) {
        RowId rid;
        rid.pageId.heapId = 1;
        rid.pageId.page_num = static_cast<u64>(i - 100);
        rid.record_num = static_cast<u64>(i);
        idx.insert(i, rid);
    }

    std::cout << "\nLooking up employee ID 105..." << std::endl;
    auto lookup = idx.lookup(105);
    if (lookup.has_value()) {
        std::cout << "Found employee at page=" << lookup->pageId.page_num
                  << ", record=" << lookup->record_num << std::endl;
    }

    std::cout << "\nRange lookup [102, 107]..." << std::endl;
    auto rangeLookup = idx.rangeLookup(102, 107);
    std::cout << "Found " << rangeLookup.size() << " employees in range" << std::endl;

    std::cout << "\n=== Demo Complete ===" << std::endl;
    return 0;
}
