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
        if (theHeapFile != nullptr) {
            return scan_heap(theHeapFile);
        }
        return std::vector<Row*>();
    }

    RowId Table::insert_row() {
        RowId rid;
        rid.pageId.heapId = 0;
        rid.pageId.page_num = 0;
        rid.record_num = 0;
        return rid;
    }

    RowId Table::insert_row(Row* row) {
        if (theHeapFile == nullptr || row == nullptr) {
            RowId rid;
            rid.pageId.heapId = 0;
            rid.pageId.page_num = 0;
            rid.record_num = 0;
            return rid;
        }
        return DB::insert_row(theHeapFile, row, 0);
    }

    Row* Table::read_row() {
        return nullptr;
    }

    Row* Table::read_row(const RowId& rid) {
        HeapFile* heapfile = get_heapfile_by_rowid(rid);
        if (heapfile == nullptr) {
            heapfile = theHeapFile;
        }
        if (heapfile == nullptr) {
            return nullptr;
        }
        return get_row(heapfile, rid);
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

