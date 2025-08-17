#pragma once

#include "Page.hpp"
#include "Types.hpp"

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
  string  table;
  string  transaction_id;
  string  filename;
  Page&   buffer;
  int     (*file_op)(string, Page&);
};