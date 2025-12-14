#pragma once

#include "general/Page.hpp"
#include "storage-manager/StorageStructs.hpp"

#include <cstring>
#include <iostream>
#include <stdint.h>
#include <vector>
#include <unordered_map>

#define SLOT_SIZE 128
#define SLOTS_PER_PAGE (PAGE_DATA_SIZE / SLOT_SIZE)
#define ROW_HEADER_SIZE (sizeof(u8) + sizeof(u8))
#define GET_PAGE_OFFSET(page_num) ((off_t)(page_num) * PAGE_DATA_SIZE)
#define GET_SLOT_OFFSET(page_num, slot_num) (GET_PAGE_OFFSET(page_num) + ((slot_num) * SLOT_SIZE))

namespace DB {
/**
 * Only allow fixed sized pages
 * Page Directory implementation of heapfiles
 */
struct HeapFile_Metadata {
  string identifier; // type heapfile and is always at page 0
  u64 heap_id;       // local to table
  u64 table_id;
  u64 num_pages;
  u64 num_records;
  u8 next_heapfile; // 0 means null
  ssize_t size = identifier.size() + 1 + sizeof(heap_id) + sizeof(table_id) +
                 sizeof(num_pages) + sizeof(num_records) +
                 sizeof(next_heapfile);
} __attribute__((packed));
inline u8 *to_bytes(HeapFile_Metadata *metadata) {
  u8 *buffer = new u8[metadata->size];
  size_t offset = 0;

  // string contents
  std::memcpy(buffer + offset, metadata->identifier.data(),
              metadata->identifier.size() + 1);
  offset += metadata->identifier.size() + 1;
  // integers
  std::memcpy(buffer + offset, &(metadata->heap_id), sizeof(metadata->heap_id));
  offset += sizeof(metadata->heap_id);
  std::memcpy(buffer + offset, &(metadata->table_id),
              sizeof(metadata->table_id));
  offset += sizeof(metadata->table_id);
  std::memcpy(buffer + offset, &(metadata->num_pages),
              sizeof(metadata->num_pages));
  offset += sizeof(metadata->num_pages);
  std::memcpy(buffer + offset, &(metadata->num_records),
              sizeof(metadata->num_records));
  offset += sizeof(metadata->num_records);
  std::memcpy(buffer + offset, &(metadata->next_heapfile),
              sizeof(metadata->next_heapfile));
  offset += sizeof(metadata->next_heapfile);

  return buffer;
}

struct HeapPageEntry {
  u32 page_id;
  u64 free_space; // number of free bytes
} __attribute__((packed));

// First page is always heapfile metadata + Table Schema + list of page ids
struct HeapFile {
  Page *get_page(u32 pid);
  // table specific functions
  // table just needs a way to get, insert, delete, update, and scan rows
  //  void                delete_row(RowId rid);

  HeapFile_Metadata metadata;
  int heap_fd;
  int num_heapfiles;
  HeapFile(int table_id, string tablename, bool if_missing);
};

HeapFile *create_heapfile(string tablename);
HeapFile *initalize_heapfile(string tablename);
HeapFile *read_heapfile();

void print_heapfile_metadata(HeapFile *heapfile);
void print_table(HeapFile heapfile);

Row *get_row(HeapFile *heapfile, RowId id);
RowId insert_row(HeapFile *heapfile, Row *row, u32 page);
RowId delete_row(HeapFile *heapfile, RowId rid);
std::vector<Row *> scan_heap(HeapFile *heapfile);

std::unordered_map<u64, HeapFile *> &get_heapfile_registry();
void register_heapfile(HeapFile *heapfile);
void unregister_heapfile(u64 heap_id);
HeapFile *get_heapfile_by_rowid(const RowId &rid);
void load_heapfile_registry(const string &tablename);

} // namespace DB
