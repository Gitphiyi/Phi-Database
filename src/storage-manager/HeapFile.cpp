#include "storage-manager/HeapFile.hpp"
#include "page-manager/DbFile.hpp"

#include <iostream>
namespace DB {
    HeapFile::HeapFile(int table_id, string tablename, bool if_missing) : metadata{tablename + "_heapfile_0",
               static_cast<u64>(0),
               static_cast<u64>(table_id),
               static_cast<u64>(0),
               static_cast<u64>(0)}, num_heapfiles(0) {
        if(if_missing) {
            std::cout << "Heapfile exists\n";
        }
        string identifier = tablename + "_heapfile_0";
        //std::cout << metadata.identifier;
        DbFile dbfile = DbFile::getInstance();
        int heapFd = dbfile.create_heapfile(tablename);
        std::byte* write_buffer = metadata.to_bytes();
        dbfile.write_at(0, &write_buffer, metadata.size, heapFd);
        delete[] write_buffer;
        std::byte* temp[metadata.size];
        dbfile.read_at(0, &temp, metadata.size, heapFd);
    }
}