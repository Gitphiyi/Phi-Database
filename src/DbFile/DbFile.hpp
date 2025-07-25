#pragma once

#include "Types.hpp"

class DbFile {
    public:
        enum LockMode { Shared, Exclusive };

        DbFile(const string& path, bool ifMissing, int permission);
        ~DbFile();

        ssize_t read_at(void* buff, off_t offset, size_t sz);
        ssize_t write_at(void* buff, off_t offset, size_t sz);

        //Force cached data and metadata to storage
        void sync();
        void lock(LockMode mode);    
        void unlock();
        void close();

    private:
        int theFd; //default fd value
};