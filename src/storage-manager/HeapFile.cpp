#include "storage-manager/HeapFile.hpp"
#include "page-manager/DbFile.hpp"

#include <iostream>
#include <unistd.h>

namespace DB {
    HeapFile::HeapFile(int table_id, string tablename, bool if_missing) : 
                metadata
                {
                    tablename + "_heapfile_1",
                    (u64) 3,
                    (u64) table_id,
                    (u64) 0,
                    (u64) 0,
                    (u8)  0x8
                }, 
                num_heapfiles(0) {
        DbFile dbfile = DbFile::getInstance();
        // if(if_missing) {
        //     std::cout << "Heapfile does not exist\n";
        // }
        int heapFd = dbfile.add_filepath("database-files/heapfiles/"+tablename+".db");
        heap_fd = heapFd;
        u8* write_buffer = to_bytes(&metadata);
        dbfile.write_at(0, write_buffer, metadata.size, heapFd);
        delete[] write_buffer;

        // Fill rest of page with page entries
        size_t remaining_bytes = PAGE_DATA_SIZE - metadata.size;
        off_t entry_offset = metadata.size;
        u8 buffer[sizeof(HeapPageEntry)];
        u64 page_id = 1;
        page_id = 0x12345678;
        u32 init_size = 0;
        memcpy(buffer+sizeof(u64), &init_size, sizeof(init_size));
        while(remaining_bytes >= sizeof(HeapPageEntry)) {
            memcpy(buffer, &page_id, sizeof(page_id));
            ++page_id;
            dbfile.write_at(entry_offset, buffer, sizeof(HeapPageEntry), heapFd);
            entry_offset += sizeof(HeapPageEntry);
            remaining_bytes -= sizeof(HeapPageEntry);
        }
    }

    void print_heapfile_metadata(HeapFile* heapfile) {
        DbFile dbfile = DbFile::getInstance();
        int i = 0;
        off_t offset = 0;
        HeapFile_Metadata read_metadata;
        std::cout << heapfile->metadata.identifier.size() + 1 << std::endl;
        char str_buffer[heapfile->metadata.identifier.size() + 1];
        dbfile.read_at(offset, &str_buffer, heapfile->metadata.identifier.size() + 1, heapfile->heap_fd);
        read_metadata.identifier.assign(str_buffer, heapfile->metadata.identifier.size() + 1);
        offset +=  heapfile->metadata.identifier.size() + 1;
        dbfile.read_at(offset, &read_metadata.heap_id, sizeof(heapfile->metadata.heap_id), heapfile->heap_fd);
        offset += sizeof(read_metadata.heap_id);
        dbfile.read_at(offset, &read_metadata.table_id, sizeof(heapfile->metadata.table_id), heapfile->heap_fd);
        offset += sizeof(read_metadata.heap_id);

        std::cout << "Heapfile Metadata: \n";
        std::cout << "  identifier = " << read_metadata.identifier << std::endl;
        std::cout << "  heap id = " << read_metadata.heap_id << std::endl;
        std::cout << "  table id = " << read_metadata.table_id << std::endl;

        //print all the pageId-capacity pairs
    }


}