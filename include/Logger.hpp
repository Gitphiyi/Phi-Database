#pragma once

#include "Types.hpp"

class Logger {
    public:
        Logger();
        ~Logger();
        void    updateLog();
        

    private: 
        string  theTableName;
        string  theLogFile;
        int     theDbFd;

        void    parseLog();
};