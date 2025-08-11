#pragma once

#include <unordered_map>

#include "Page.hpp"
#include "Types.hpp"

namespace DB {
    class DbFile {
        public:
            enum LockMode { Shared, Exclusive };

            DbFile(const string& path, bool ifMissing);
            ~DbFile();

            static void initialize(const string& path, bool ifMissing);
            static DbFile& getInstance();
            static void checkIfFileDescriptorValid(int aFd);
            
            //page offset and buffer page to read/write
            ssize_t db_read_at(off_t offset, Page& buffer);
            ssize_t read_at(off_t offset, Page& buffer, int fd);
            ssize_t db_write_at(off_t offset, Page& buffer);
            ssize_t write_at(off_t offset, Page& buffer, int fd);
            int     get_path(const string& path);
            int     add_path(const string& path);

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