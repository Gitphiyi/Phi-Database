#pragma once

#include <unordered_map>
#include <stack>


#include "Types.hpp"
#include "DbFile.hpp"
#include "Page.hpp"

namespace DB {
    class PageCache {
        public:
            const u64 CACHE_SIZE;
            const u32 NUM_PAGES;
            PageCache(u32 numPages, DbFile& fileApi);

            Page&                               read(u32 pageId, Page& buffer, const string& filepath); 
            bool                                write_through(Page& page, const string& filepath); // write through
            void                                print();
        private:
            DbFile&                             theDbFile;
            std::unordered_map<u32, Page*>      thePageMap;
            std::stack<Page*>                   theUsedPages;
            void                                evict_add_page(Page& page);
            //might not need thePages. will do optimizations later
    };
}