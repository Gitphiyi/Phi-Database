#include "storage-manager/HeapFile.hpp"
#include "page-manager/DbFile.hpp"

#include <cstring>
#include <iostream>
#include <unistd.h>

namespace DB {

static size_t serialize_row(Row *row, u8 *buffer, size_t buffer_size);
static Row *deserialize_row(u8 *buffer, size_t size);

HeapFile::HeapFile(int table_id, string tablename, bool if_missing)
    : metadata{tablename + "_heapfile_1",
               (u64)3,
               (u64)table_id,
               (u64)0,
               (u64)0,
               (u8)0x8},
      num_heapfiles(0) {
  DbFile dbfile = DbFile::getInstance();
  if (if_missing) {
    std::cout << "Heapfile does not exist\n";
  }
  int heapFd =
      dbfile.add_filepath("database-files/heapfiles/" + tablename + ".db");
  heap_fd = heapFd;
  u8 *write_buffer = to_bytes(&metadata);
  dbfile.write_at(0, write_buffer, metadata.size, heapFd);
  delete[] write_buffer;

  // Fill rest of page with page entries
  size_t remaining_bytes = PAGE_DATA_SIZE - metadata.size;
  off_t entry_offset = metadata.size;
  u8 buffer[sizeof(HeapPageEntry)];
  u64 page_id = 1;
  page_id = 0x12345678;
  u32 init_size = 1;
  memcpy(buffer + sizeof(u64), &init_size, sizeof(init_size));
  int num_pages = 0;
  while (remaining_bytes >= sizeof(HeapPageEntry)) {
    memcpy(buffer, &page_id, sizeof(page_id));
    ++page_id;
    std::cout << "entry offset = " << entry_offset << std::endl;
    dbfile.write_at(entry_offset, buffer, sizeof(HeapPageEntry), heapFd);
    entry_offset += sizeof(HeapPageEntry);
    remaining_bytes -= sizeof(HeapPageEntry);
    num_pages++;
  }
  std::cout << "There are " << num_pages << " # of pages on the first page \n";
}
HeapFile *create_heapfile(string tablename) {
  HeapFile *heapfile = new HeapFile(1, tablename, true);
  return heapfile;
}

void print_heapfile_metadata(HeapFile *heapfile) {
  DbFile dbfile = DbFile::getInstance();
  int i = 0;
  off_t offset = 0;
  HeapFile_Metadata read_metadata;
  read_metadata.size = heapfile->metadata.size;

  char str_buffer[heapfile->metadata.identifier.size() + 1];
  dbfile.read_at(offset, &str_buffer, heapfile->metadata.identifier.size() + 1,
                 heapfile->heap_fd);
  read_metadata.identifier.assign(str_buffer,
                                  heapfile->metadata.identifier.size() + 1);
  offset += heapfile->metadata.identifier.size() + 1;

  dbfile.read_at(offset, &read_metadata.heap_id,
                 sizeof(heapfile->metadata.heap_id), heapfile->heap_fd);
  offset += sizeof(read_metadata.heap_id);

  dbfile.read_at(offset, &read_metadata.table_id,
                 sizeof(heapfile->metadata.table_id), heapfile->heap_fd);
  offset += sizeof(read_metadata.table_id);

  std::cout << "Heapfile Metadata: \n";
  std::cout << "  identifier = " << read_metadata.identifier << std::endl;
  std::cout << "  heap id = " << read_metadata.heap_id << std::endl;
  std::cout << "  table id = " << read_metadata.table_id << std::endl;

  // print all the pageId capacity pairs
  size_t remaining_bytes = PAGE_DATA_SIZE - read_metadata.size;
  offset = read_metadata.size;

  int num = 1;
  while (remaining_bytes >= sizeof(HeapPageEntry)) {
    HeapPageEntry entry;
    dbfile.read_at(offset, &entry, sizeof(HeapPageEntry), heapfile->heap_fd);
    offset += sizeof(HeapPageEntry);
    remaining_bytes -= sizeof(HeapPageEntry);
    std::cout << "Row Entry " << num << "-> page id = " << entry.page_id
              << "\t\tfree space (B) = " << entry.free_space << std::endl;
    num++;
  }
}

static size_t serialize_row(Row *row, u8 *buffer, size_t buffer_size) {
  if (row == NULL || buffer == NULL) {
    return 0;
  }

  size_t offset = 0;
  u8 valid_marker = 1;
  memcpy(buffer + offset, &valid_marker, sizeof(u8));
  offset += sizeof(u8);

  memcpy(buffer + offset, &row->numCols, sizeof(u8));
  offset += sizeof(u8);

  for (size_t i = 0; i < row->numCols && offset < buffer_size; i++) {
    u8 type_tag = (u8)row->values[i].index();
    memcpy(buffer + offset, &type_tag, sizeof(u8));
    offset += sizeof(u8);

    // encodes the column type since they are variant type
    switch (row->values[i].index()) {
    case 0: {
      int val = std::get<int>(row->values[i]);
      memcpy(buffer + offset, &val, sizeof(int));
      offset += sizeof(int);
      break;
    }
    case 1: {
      float val = std::get<float>(row->values[i]);
      memcpy(buffer + offset, &val, sizeof(float));
      offset += sizeof(float);
      break;
    }
    case 2: {
      string val = std::get<string>(row->values[i]);
      u16 len = (u16)val.size();
      memcpy(buffer + offset, &len, sizeof(u16));
      offset += sizeof(u16);
      memcpy(buffer + offset, val.data(), len);
      offset += len;
      break;
    }
    case 3: {
      bool val = std::get<bool>(row->values[i]);
      memcpy(buffer + offset, &val, sizeof(bool));
      offset += sizeof(bool);
      break;
    }
    case 4: {
      int64_t val = std::get<int64_t>(row->values[i]);
      memcpy(buffer + offset, &val, sizeof(int64_t));
      offset += sizeof(int64_t);
      break;
    }
    case 5: {
      double val = std::get<double>(row->values[i]);
      memcpy(buffer + offset, &val, sizeof(double));
      offset += sizeof(double);
      break;
    }
    }
  }

  return offset;
}

static Row *deserialize_row(u8 *buffer, size_t size) {
  if (buffer == NULL || size < ROW_HEADER_SIZE) {
    return NULL;
  }

  size_t offset = 0;
  u8 valid_marker;
  memcpy(&valid_marker, buffer + offset, sizeof(u8));
  offset += sizeof(u8);

  if (valid_marker == 0) {
    return NULL;
  }

  u8 num_cols;
  memcpy(&num_cols, buffer + offset, sizeof(u8));
  offset += sizeof(u8);

  std::vector<datatype> values;
  values.reserve(num_cols);

  for (u8 i = 0; i < num_cols && offset < size; i++) {
    u8 type_tag;
    memcpy(&type_tag, buffer + offset, sizeof(u8));
    offset += sizeof(u8);

    switch (type_tag) {
    case 0: {
      int val;
      memcpy(&val, buffer + offset, sizeof(int));
      offset += sizeof(int);
      values.push_back(val);
      break;
    }
    case 1: {
      float val;
      memcpy(&val, buffer + offset, sizeof(float));
      offset += sizeof(float);
      values.push_back(val);
      break;
    }
    case 2: {
      u16 len;
      memcpy(&len, buffer + offset, sizeof(u16));
      offset += sizeof(u16);
      string val((char *)(buffer + offset), len);
      offset += len;
      values.push_back(val);
      break;
    }
    case 3: {
      bool val;
      memcpy(&val, buffer + offset, sizeof(bool));
      offset += sizeof(bool);
      values.push_back(val);
      break;
    }
    case 4: {
      int64_t val;
      memcpy(&val, buffer + offset, sizeof(int64_t));
      offset += sizeof(int64_t);
      values.push_back(val);
      break;
    }
    case 5: {
      double val;
      memcpy(&val, buffer + offset, sizeof(double));
      offset += sizeof(double);
      values.push_back(val);
      break;
    }
    }
  }

  Row *row = new Row(num_cols, std::move(values));
  return row;
}

Row *get_row(HeapFile *heapfile, RowId rid) {
  if (heapfile == NULL) {
    return NULL;
  }

  DbFile &dbfile = DbFile::getInstance();
  u8 buffer[SLOT_SIZE];
  off_t slot_off = GET_SLOT_OFFSET((u32)rid.pageId.page_num, rid.record_num);

  ssize_t bytes_read =
      dbfile.read_at(slot_off, buffer, SLOT_SIZE, heapfile->heap_fd);
  if (bytes_read <= 0) {
    return NULL;
  }

  return deserialize_row(buffer, SLOT_SIZE);
}

RowId insert_row(HeapFile *heapfile, Row *row, u32 page_num) {
  RowId rid = {};
  rid.pageId.heapId = 0;
  rid.pageId.page_num = 0;
  rid.record_num = 0;

  if (heapfile == NULL || row == NULL) {
    return rid;
  }

  DbFile &dbfile = DbFile::getInstance();

  if (page_num == 0) {
    page_num = 1;
  }

  u8 buffer[SLOT_SIZE];
  memset(buffer, 0, SLOT_SIZE);

  off_t page_start = GET_PAGE_OFFSET(page_num);
  u64 slot_num = 0;
  bool found_slot = false;

  for (u64 i = 0; i < SLOTS_PER_PAGE; i++) {
    off_t slot_off = page_start + (i * SLOT_SIZE);
    u8 valid_marker = 0;
    dbfile.read_at(slot_off, &valid_marker, sizeof(u8), heapfile->heap_fd);

    if (valid_marker == 0) {
      slot_num = i;
      found_slot = true;
      break;
    }
  }

  if (!found_slot) {
    page_num++;
    slot_num = 0;
  }

  memset(buffer, 0, SLOT_SIZE);
  size_t row_size = serialize_row(row, buffer, SLOT_SIZE);

  if (row_size == 0) {
    return rid;
  }

  off_t write_off = GET_SLOT_OFFSET(page_num, slot_num);
  dbfile.write_at(write_off, buffer, SLOT_SIZE, heapfile->heap_fd);

  heapfile->metadata.num_records++;

  rid.pageId.heapId = heapfile->metadata.heap_id;
  rid.pageId.page_num = page_num;
  rid.record_num = slot_num;

  return rid;
}

RowId delete_row(HeapFile *heapfile, RowId rid) {
  RowId result = {};
  result.pageId.heapId = 0;
  result.pageId.page_num = 0;
  result.record_num = 0;

  if (heapfile == NULL) {
    return result;
  }

  DbFile &dbfile = DbFile::getInstance();
  off_t slot_off = GET_SLOT_OFFSET((u32)rid.pageId.page_num, rid.record_num);

  u8 zero_marker = 0;
  dbfile.write_at(slot_off, &zero_marker, sizeof(u8), heapfile->heap_fd);

  if (heapfile->metadata.num_records > 0) {
    heapfile->metadata.num_records--;
  }

  result = rid;
  return result;
}

static std::unordered_map<u64, HeapFile *> heapfile_registry;

std::unordered_map<u64, HeapFile *> &get_heapfile_registry() {
  return heapfile_registry;
}

void register_heapfile(HeapFile *heapfile) {
  if (heapfile != NULL) {
    heapfile_registry[heapfile->metadata.heap_id] = heapfile;
  }
}

void unregister_heapfile(u64 heap_id) { heapfile_registry.erase(heap_id); }

HeapFile *get_heapfile_by_rowid(const RowId &rid) {
  u64 heap_id = rid.pageId.heapId;
  auto it = heapfile_registry.find(heap_id);
  if (it != heapfile_registry.end()) {
    return it->second;
  }
  return NULL;
}

static HeapFile *read_heapfile_from_disk(const string &filepath, int heap_num) {
  DbFile &dbfile = DbFile::getInstance();
  int fd = dbfile.add_filepath(filepath);
  if (fd < 0) {
    return NULL;
  }

  char id_buffer[256];
  off_t offset = 0;
  ssize_t bytes_read = dbfile.read_at(offset, id_buffer, sizeof(id_buffer), fd);
  if (bytes_read <= 0) {
    return NULL;
  }

  size_t id_len = strlen(id_buffer);
  offset = id_len + 1;

  HeapFile *heapfile = new HeapFile(0, "", false);
  heapfile->heap_fd = fd;
  heapfile->metadata.identifier = string(id_buffer, id_len);

  dbfile.read_at(offset, &heapfile->metadata.heap_id,
                 sizeof(heapfile->metadata.heap_id), fd);
  offset += sizeof(heapfile->metadata.heap_id);

  dbfile.read_at(offset, &heapfile->metadata.table_id,
                 sizeof(heapfile->metadata.table_id), fd);
  offset += sizeof(heapfile->metadata.table_id);

  dbfile.read_at(offset, &heapfile->metadata.num_pages,
                 sizeof(heapfile->metadata.num_pages), fd);
  offset += sizeof(heapfile->metadata.num_pages);

  dbfile.read_at(offset, &heapfile->metadata.num_records,
                 sizeof(heapfile->metadata.num_records), fd);
  offset += sizeof(heapfile->metadata.num_records);

  dbfile.read_at(offset, &heapfile->metadata.next_heapfile,
                 sizeof(heapfile->metadata.next_heapfile), fd);

  return heapfile;
}

void load_heapfile_registry(const string &tablename) {
  int heap_num = 1;
  string filepath = "database-files/heapfiles/" + tablename + ".db";

  HeapFile *heapfile = read_heapfile_from_disk(filepath, heap_num);
  while (heapfile != NULL) {
    register_heapfile(heapfile);

    if (heapfile->metadata.next_heapfile == 0) {
      break;
    }

    heap_num++;
    filepath = "database-files/heapfiles/" + tablename + "_" +
               std::to_string(heap_num) + ".db";
    heapfile = read_heapfile_from_disk(filepath, heap_num);
  }
}

std::vector<Row *> scan_heap(HeapFile *heapfile) {
  std::vector<Row *> rows;

  if (heapfile == NULL) {
    return rows;
  }

  DbFile &dbfile = DbFile::getInstance();
  u8 buffer[SLOT_SIZE];

  u32 max_pages = 100;
  for (u32 page_num = 1; page_num < max_pages; page_num++) {
    bool page_empty = true;

    for (u64 slot_num = 0; slot_num < SLOTS_PER_PAGE; slot_num++) {
      off_t slot_off = GET_SLOT_OFFSET(page_num, slot_num);
      ssize_t bytes_read =
          dbfile.read_at(slot_off, buffer, SLOT_SIZE, heapfile->heap_fd);

      if (bytes_read <= 0) {
        continue;
      }

      if (buffer[0] != 0) {
        page_empty = false;
        Row *row = deserialize_row(buffer, SLOT_SIZE);
        if (row != NULL) {
          rows.push_back(row);
        }
      }
    }

    if (page_empty && page_num > 1) {
      break;
    }
  }

  return rows;
}

} // namespace DB
