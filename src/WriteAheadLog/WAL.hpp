#pragma once

#include "Types.hpp"

//Write Ahead Logger
class WAL {
    public:
        WAL(string tableName);
        ~WAL();
        void    updateLog();
        

    private: 
        string  theTableName;
        string  theLogFile;
        int     theFd;

        void    parseLog();  
};