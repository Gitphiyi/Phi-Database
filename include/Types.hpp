#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <variant> 

using u64 = std::uint64_t;
using u32 = std::uint32_t;
using u16 = std::uint16_t;
using u8  = std::uint8_t;
using size_t = std::size_t;
using string = std::string;
using datatype = std::variant<int, float, std::string, bool, int64_t>;


enum Orientation {
    COLUMN,
    ROW
};

struct SchemaCol {
    string      name;
    datatype   type;
    bool        nullable;
    datatype    default_val;
    bool        is_primary_key;
    bool        is_unique;

    SchemaCol(const string& name, datatype type, bool nullable = true,
        bool primaryKey = false, bool unique = false, datatype defaultVal = {}) : 
        name(name),
        type(type),
        nullable(nullable),
        is_primary_key(primaryKey),
        is_unique(unique),
        default_val(defaultVal) {}
};

struct Schema {
  std::vector<SchemaCol> columns;

  explicit Schema(size_t reserveCount) {
    columns.reserve(reserveCount);
  }

  void add_col(const std::string& name,
               datatype       type,
               bool           nullable      = true,
               bool           primaryKey    = false,
               bool           unique        = false,
               datatype       defaultVal    = {}) 
  {
    columns.emplace_back(name, type, nullable, primaryKey, unique, defaultVal);
  }
};

struct Operation {
  string  filename;
  Page&   buffer;
  int     (*file_op)(string, Page&);
};