#pragma once

#include <unordered_map>
#include <stack>

#include "DbFile.hpp"
#include "Page.hpp"

namespace DB {
    class PageCache {
        public:
            const u64 CACHE_SIZE; //
            const u32 NUM_PAGES;
            PageCache(u32 numPages, DbFile fileApi);
            ~PageCache();

            Page& read(u32 pageId); 
            int write_through(); // write through
            bool loadPageFromDisk(u32 pageId, Page& page);

        private:
            DbFile                              theDbFile;
            std::unordered_map<u32, Page*>      thePageMap;
            std::stack<Page*>                   theFreePages;
            Page*                               thePages; //array of pages to ensure pages are in memory
    };
}