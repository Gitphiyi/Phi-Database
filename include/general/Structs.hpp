#pragma once

#include <vector>

#include "Page.hpp"
#include "Types.hpp"

struct SchemaCol {
    string      name;
    ColumnType   type;
    bool        nullable;
    datatype    default_val;
    bool        is_primary_key;
    bool        is_unique;

    SchemaCol(const string& name, ColumnType type, bool nullable = true,
        bool primaryKey = false, bool unique = false, datatype defaultVal = {}) : 
        name(name),
        type(type),
        nullable(nullable),
        is_primary_key(primaryKey),
        is_unique(unique),
        default_val(defaultVal) {}
    
    size_t get_size() {
      size_t sz = 0;
      sz += name.length() + 1;
      sz += sizeof(type) + sizeof(nullable) + sizeof(default_val) + sizeof(is_primary_key) + sizeof(is_unique);
      return sz;
    }
};

struct Schema {
  std::vector<SchemaCol> columns;

  explicit Schema(size_t reserveCount) {
    columns.reserve(reserveCount);
  }

  void add_col(const std::string& name,
               ColumnType         type,
               bool           nullable      = true,
               bool           primaryKey    = false,
               bool           unique        = false,
               datatype       defaultVal    = {}) 
  {
    columns.emplace_back(name, type, nullable, primaryKey, unique, defaultVal);
  }
  void print() {
    std::cout << "| ";
    for(SchemaCol col : columns) {
      std::cout << col.name;
      if(col.is_primary_key) {
        std::cout<<"(PRIMARY) ";
      }
        if(col.is_unique) {
        std::cout<<"(UNIQUE) ";
      }
      std::cout<< " | ";
    }
    std::cout<<std::endl;
  }

  size_t get_size() {
    size_t sz = 0;
    for(SchemaCol col : columns) {
      sz += col.get_size();
    }
    return sz;
  }
};