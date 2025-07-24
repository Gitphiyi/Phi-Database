#pragma once

#include "Types.hpp"

class DbFile {
    public:
        enum LockMode { Shared, Exclusive };

        DbFile(const string& path, bool ifMissing, int permission);
        ~DbFile();

        size_t read_at();
        size_t write_at();
        //Force cached data and metadata to storage
        void sync();
        void lock(LockMode mode);    
        void unlock();
        void close();

    private:
        int theFd; //default fd value
};