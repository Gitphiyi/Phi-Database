#pragma once

#include "general/Types.hpp"
#include "general/Structs.hpp"
#include <vector>

struct PageId {
    u64     heapId;
    u64     page_num; // offset in heap
};

struct RowId {
    PageId      pageId;
    u64         record_num; // index in page
};

struct Row {
    //RowId       id; // for lazy allocation when needed
    u8          numCols;
    std::vector<datatype>   values;
    Row(int n, std::vector<datatype>&& v) : numCols(n), values(std::move(v)) {}
};

struct TableId {
    u64 id;
};

struct TableMetadataHeader {
  string name;
  Schema& schema;
  TableMetadataHeader(const string& Name, Schema& Schema) : name(Name), schema(Schema) {}

  size_t get_size() {
    size_t sz = 0;
    sz += name.length() + 1; //add terminator byte at end
    sz += schema.get_size();
    return sz;
  }
};