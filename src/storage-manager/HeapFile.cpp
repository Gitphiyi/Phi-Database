#include "storage-manager/HeapFile.hpp"
#include "page-manager/DbFile.hpp"

#include <iostream>
#include <unistd.h>

namespace DB {
    HeapFile::HeapFile(int table_id, string tablename, bool if_missing) : 
                metadata
                {
                    tablename + "_heapfile",
                    (u64) 3,
                    (u64) table_id,
                    (u64) 0,
                    (u64) 0,
                    (u8)  0
                }, 
                num_heapfiles(0) {
        DbFile dbfile = DbFile::getInstance();
        // if(if_missing) {
        //     std::cout << "Heapfile does not exist\n";
        // }
        int heapFd = dbfile.add_filepath("database-files/heapfiles/"+tablename+".db");
        std::byte* write_buffer = to_bytes(&metadata);

        size_t i;
        std::cout << "identifier sz = " << metadata.identifier.size() <<std::endl;
        std::cout << "\nread from write buffer \n";
        for(i = 0; i < metadata.identifier.size()+1; i++) {
            std::cout << (char) write_buffer[i];
        }
        for (; i + sizeof(uint64_t) <= metadata.size; i += sizeof(uint64_t)) {
            uint64_t val = 0;
            memcpy(&val, write_buffer + i, sizeof(val));   // safe and portable
            printf("offset 0x%04zx : 0x%016llx\n", i, (unsigned long long)val);
        }


        dbfile.write_at(0, write_buffer, metadata.size, heapFd);
        delete[] write_buffer;
        std::byte temp[metadata.size];
        dbfile.read_at(0, temp, metadata.size, heapFd);
    }

    void read
}