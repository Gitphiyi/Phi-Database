#pragma once

#include <unordered_map>
#include <stack>

#include "Page.hpp"

class PageCache {
    public:
        const u64 BUFFER_SIZE;
        const u32 NUM_PAGES;
        PageCache(u32 numPages);
        ~PageCache();

        int read(); 
        int write_through(); // write through

    private:
        std::unordered_map<u32, Page*>      thePageMap;
        std::stack<Page*>                   theFreePages;
        Page*                               thePages; //array of pages to ensure pages are in memory
};