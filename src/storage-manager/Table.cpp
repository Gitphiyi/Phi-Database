#include "storage-manager/Table.hpp"
#include "page-manager/DbFile.hpp"

#include <sys/fcntl.h>
#include <cstring>

namespace DB {
    Table::Table(const string& name,
            Schema& schema,
            HeapFile& heapfile,
            PageCache* pageCache
            ) :
        theFileName(name),
        thePath("db/table/"+name),
        theSchema(schema),
        theHeapFile(&heapfile),
        thePageCache(pageCache)
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
        RowId rid = {};
        rid.pageId.heapId = 0;
        rid.pageId.page_num = 0;
        rid.record_num = 0;

        if (theHeapFile == nullptr || row == nullptr) {
            return rid;
        }

        DbFile& dbfile = DbFile::getInstance();
        string filepath = "database-files/heapfiles/" + theFileName + ".db";

        u32 page_num = 1;  // Start from page 1 (page 0 is metadata)

        // Check if page is in cache first
        if (thePageCache != nullptr) {
            Page buffer;
            buffer.id = page_num;
            Page& cachedPage = thePageCache->read(page_num, buffer, filepath);

            // Find a free slot in the cached page
            u64 slot_num = 0;
            bool found_slot = false;

            for (u64 i = 0; i < SLOTS_PER_PAGE; i++) {
                u8* slot_data = reinterpret_cast<u8*>(cachedPage.data) + (i * SLOT_SIZE);
                if (slot_data[0] == 0) {  // Check valid marker
                    slot_num = i;
                    found_slot = true;
                    break;
                }
            }

            if (found_slot) {
                // Serialize the row
                u8 row_buffer[SLOT_SIZE];
                std::memset(row_buffer, 0, SLOT_SIZE);

                // Serialize row: valid marker + numCols + values
                size_t offset = 0;
                u8 valid_marker = 1;
                std::memcpy(row_buffer + offset, &valid_marker, sizeof(u8));
                offset += sizeof(u8);
                std::memcpy(row_buffer + offset, &row->numCols, sizeof(u8));
                offset += sizeof(u8);

                for (size_t i = 0; i < row->numCols && offset < SLOT_SIZE; i++) {
                    u8 type_tag = (u8)row->values[i].index();
                    std::memcpy(row_buffer + offset, &type_tag, sizeof(u8));
                    offset += sizeof(u8);

                    switch (row->values[i].index()) {
                        case 0: { // int
                            int val = std::get<int>(row->values[i]);
                            std::memcpy(row_buffer + offset, &val, sizeof(int));
                            offset += sizeof(int);
                            break;
                        }
                        case 1: { // float
                            float val = std::get<float>(row->values[i]);
                            std::memcpy(row_buffer + offset, &val, sizeof(float));
                            offset += sizeof(float);
                            break;
                        }
                        case 2: { // string
                            string val = std::get<string>(row->values[i]);
                            u16 len = (u16)val.size();
                            std::memcpy(row_buffer + offset, &len, sizeof(u16));
                            offset += sizeof(u16);
                            std::memcpy(row_buffer + offset, val.data(), len);
                            offset += len;
                            break;
                        }
                        case 3: { // bool
                            bool val = std::get<bool>(row->values[i]);
                            std::memcpy(row_buffer + offset, &val, sizeof(bool));
                            offset += sizeof(bool);
                            break;
                        }
                        case 4: { // int64_t
                            int64_t val = std::get<int64_t>(row->values[i]);
                            std::memcpy(row_buffer + offset, &val, sizeof(int64_t));
                            offset += sizeof(int64_t);
                            break;
                        }
                        case 5: { // double
                            double val = std::get<double>(row->values[i]);
                            std::memcpy(row_buffer + offset, &val, sizeof(double));
                            offset += sizeof(double);
                            break;
                        }
                    }
                }

                // Copy serialized row into cached page
                u8* slot_data = reinterpret_cast<u8*>(cachedPage.data) + (slot_num * SLOT_SIZE);
                std::memcpy(slot_data, row_buffer, SLOT_SIZE);
                cachedPage.dirty_bit = true;

                // Write through to disk
                thePageCache->write_through(cachedPage, filepath);

                theHeapFile->metadata.num_records++;

                rid.pageId.heapId = theHeapFile->metadata.heap_id;
                rid.pageId.page_num = page_num;
                rid.record_num = slot_num;
                return rid;
            }
        }

        // Fallback to direct HeapFile insert if no cache or no slot found
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

        string filepath = "database-files/heapfiles/" + theFileName + ".db";
        u32 page_num = (u32)rid.pageId.page_num;
        u64 slot_num = rid.record_num;

        // Check PageCache first
        if (thePageCache != nullptr) {
            Page buffer;
            buffer.id = page_num;
            Page& cachedPage = thePageCache->read(page_num, buffer, filepath);

            // Get the slot data from the cached page
            u8* slot_data = reinterpret_cast<u8*>(cachedPage.data) + (slot_num * SLOT_SIZE);

            // Deserialize the row from slot data
            if (slot_data[0] == 0) {  // Check valid marker
                return nullptr;
            }

            size_t offset = 0;
            offset += sizeof(u8);  // Skip valid marker

            u8 num_cols;
            std::memcpy(&num_cols, slot_data + offset, sizeof(u8));
            offset += sizeof(u8);

            std::vector<datatype> values;
            values.reserve(num_cols);

            for (u8 i = 0; i < num_cols && offset < SLOT_SIZE; i++) {
                u8 type_tag;
                std::memcpy(&type_tag, slot_data + offset, sizeof(u8));
                offset += sizeof(u8);

                switch (type_tag) {
                    case 0: { // int
                        int val;
                        std::memcpy(&val, slot_data + offset, sizeof(int));
                        offset += sizeof(int);
                        values.push_back(val);
                        break;
                    }
                    case 1: { // float
                        float val;
                        std::memcpy(&val, slot_data + offset, sizeof(float));
                        offset += sizeof(float);
                        values.push_back(val);
                        break;
                    }
                    case 2: { // string
                        u16 len;
                        std::memcpy(&len, slot_data + offset, sizeof(u16));
                        offset += sizeof(u16);
                        string val(reinterpret_cast<char*>(slot_data + offset), len);
                        offset += len;
                        values.push_back(val);
                        break;
                    }
                    case 3: { // bool
                        bool val;
                        std::memcpy(&val, slot_data + offset, sizeof(bool));
                        offset += sizeof(bool);
                        values.push_back(val);
                        break;
                    }
                    case 4: { // int64_t
                        int64_t val;
                        std::memcpy(&val, slot_data + offset, sizeof(int64_t));
                        offset += sizeof(int64_t);
                        values.push_back(val);
                        break;
                    }
                    case 5: { // double
                        double val;
                        std::memcpy(&val, slot_data + offset, sizeof(double));
                        offset += sizeof(double);
                        values.push_back(val);
                        break;
                    }
                }
            }

            return new Row(num_cols, std::move(values));
        }

        // Fallback to direct HeapFile read if no cache
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

    Page* Table::getPageFromCache(u32 pageId) {
        if (thePageCache == nullptr || theHeapFile == nullptr) {
            return nullptr;
        }

        string filepath = "database-files/heapfiles/" + theFileName + ".db";
        Page* buffer = new Page();
        buffer->id = pageId;
        thePageCache->read(pageId, *buffer, filepath);
        return buffer;
    }
}

