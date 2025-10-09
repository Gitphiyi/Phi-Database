#pragma once

#include <unordered_map>

#include "general/Page.hpp"
#include "general/Types.hpp"

namespace DB {
    class DbFile {
        public:
            enum LockMode { Shared, Exclusive };

            DbFile(bool ifMissing);
            ~DbFile();

            static void initialize(bool ifMissing);
            static DbFile& getInstance();
            static void checkIfFileDescriptorValid(int aFd);
            
            //page offset and buffer page to read/write
            ssize_t db_read_at(off_t offset, Page& buffer);
            ssize_t read_at(off_t offset, Page& buffer, int fd);
            ssize_t read_at(off_t offset, void* buffer, ssize_t num_bytes, int fd);
            ssize_t write_at(off_t offset, void* buffer, ssize_t num_bytes, int fd);
            ssize_t db_write_at(off_t offset, Page& buffer);
            ssize_t write_at(off_t offset, Page& buffer, int fd);
            int     get_filepath(const string& path); //return fd and -1 on failure
            int     add_filepath(const string& path, const string& name, const string& filetype);

            //Force cached data and metadata to storage
            void sync();
            void lock(LockMode mode);    
            void unlock();
            void close();

        private:
            int                             theDbFd; //db fd value
            std::unordered_map<string, int> theFdMap;
    };
}