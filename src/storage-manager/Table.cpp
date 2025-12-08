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

    Row* Table::read_row() {
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
}

