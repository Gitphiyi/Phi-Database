#pragma once

#include <unordered_map>
#include <vector>
#include <unordered_set>
#include <stack>

#include "general/Types.hpp"
#include "general/Structs.hpp"
#include "general/Page.hpp"
#include "DbFile.hpp"

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
            std::unordered_map<u32, size_t>     thePageMap;
            std::stack<size_t>                  theUsedPages;
            Page*                               theCachePages;
            std::vector<size_t>                 theFreePages;
            void                                evict_add_page(Page& page);
    };
}