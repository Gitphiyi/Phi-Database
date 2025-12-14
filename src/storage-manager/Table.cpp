#include "storage-manager/Table.hpp"

#include <sys/fcntl.h>

namespace DB {
    Table::Table(const string& name,
            Schema& schema,
            HeapFile& heapfile
            ) :
        theFileName(name),
        thePath("db/table/"+name),
        theSchema(schema),
        theHeapFile(&heapfile)
        {}

    std::vector<Row*> Table::scan() const {
        std::vector<Row*> batch;
        for (int i = 0; i < 100; ++i) {
            string s = "hi";
            std::vector<datatype> data = {s, false, i, 2.0};
            Row* r = new Row(4, std::move(data));
            batch.push_back(r);
        }
        return batch;
    }

    RowId Table::insert_row() {
        RowId rid;
        rid.pageId.heapId = 0;
        rid.pageId.page_num = 0;
        rid.record_num = 0;
        return rid;
    }

    RowId Table::insert_row(Row* row) {
        RowId rid = insert_row();

        for (auto& [colIdx, index] : theIndexes) {
            if (colIdx < static_cast<int>(row->values.size())) {
                index->insert(row->values[colIdx], rid);
            }
        }

        return rid;
    }

    Row* Table::read_row() {
        return nullptr;
    }

    Row* Table::read_row(const RowId& rid) {
        return nullptr;
    }

    u64 Table::read(u64 pageNum, u16 rowNum) {
        return 0;
    }

    string Table::print_metadata() {
        string result = "Table: " + theFileName + "\n";
        result += "Schema:\n";
        for (const auto& col : theSchema.columns) {
            result += "  " + col.name + "\n";
        }
        return result;
    }

    Schema* Table::get_record(int rid) {
        return nullptr;
    }

    void Table::createIndex(const string& indexName, int columnIndex) {
        if (theIndexes.find(columnIndex) != theIndexes.end()) {
            return;
        }
        auto index = std::make_unique<Index>(indexName, theFileName, columnIndex);
        theIndexes[columnIndex] = std::move(index);
    }

    void Table::dropIndex(const string& indexName) {
        for (auto it = theIndexes.begin(); it != theIndexes.end(); ++it) {
            if (it->second->getName() == indexName) {
                theIndexes.erase(it);
                return;
            }
        }
    }

    bool Table::hasIndex(int columnIndex) const {
        return theIndexes.find(columnIndex) != theIndexes.end();
    }

    Index* Table::getIndex(int columnIndex) {
        auto it = theIndexes.find(columnIndex);
        if (it != theIndexes.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    std::vector<RowId> Table::indexLookup(int columnIndex, const datatype& key) {
        auto* index = getIndex(columnIndex);
        if (!index) {
            return {};
        }
        return index->lookupAll(key);
    }

    std::vector<RowId> Table::indexRangeLookup(int columnIndex, const datatype& low, const datatype& high) {
        auto* index = getIndex(columnIndex);
        if (!index) {
            return {};
        }
        return index->rangeLookup(low, high);
    }
}

