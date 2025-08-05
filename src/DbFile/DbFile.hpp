#pragma once

#include "Page.hpp"
#include "Types.hpp"

namespace DB {
    class DbFile {
        public:
            enum LockMode { Shared, Exclusive };

            DbFile(const string& path, bool ifMissing, Page& page);
            ~DbFile();

            static void initialize(const std::string& path, bool ifMissing, Page& page);
            static DbFile& getInstance();
            static void checkIfFileDescriptorValid(int aFd);
            
            //page offset and buffer page to read/write
            ssize_t read_at(off_t offset);
            ssize_t read_at(Page& buffer, off_t offset);
            ssize_t write_at(off_t offset);

            //Force cached data and metadata to storage
            void sync();
            void lock(LockMode mode);    
            void unlock();
            void close();

        private:
            int         theFd; //default fd value
            Page&       theBuffer;
    };
}