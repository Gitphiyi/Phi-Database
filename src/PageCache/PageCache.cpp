#include <unordered_map>
#include <stack>
#include <iostream>
#include <new>

#include "Page.hpp"
#include "DbFile/DbFile.hpp"
#include "PageCache.hpp"

namespace DB {
    PageCache::PageCache(u32 numPages, DbFile fileApi)  : 
        BUFFER_SIZE(numPages * sizeof(Page)),
        NUM_PAGES(numPages),
        theDbFile(fileApi)
    {    
        thePages = static_cast<Page*>(::operator new[](NUM_PAGES * sizeof(Page))); //allocates space for the pages
        thePageMap.reserve(NUM_PAGES); //reserve pages to avoid rehashing

        //all pages in cache are free at the moment
        for (u32 i = 0; i < NUM_PAGES; ++i) {
            theFreePages.push(&thePages[i]);
        }
    };

    PageCache::~PageCache() {
        ::operator delete[](thePages);    
        //other objects are RAII so are destructed when class is destroyed
    }

    Page& PageCache::read(u32 pageId) {
        auto mapEntry = thePageMap.find(pageId);
        if(mapEntry == thePageMap.end() || !mapEntry->second->valid_bit) {
            std::cout << "read miss" << std::endl;

            //Step 1: get a free page
            Page* page = nullptr;
            //check if there are free pages
            if (!theFreePages.empty()) {
                page = theFreePages.top();
                theFreePages.pop();
            } else {
                if (thePageMap.empty()) {
                    throw std::runtime_error("Page cache is empty and cannot evict!");
                }
                auto victim = thePageMap.begin();
                page = victim->second;
                thePageMap.erase(victim);
                // Here, you might write back dirty pages, etc.
            }

            // Step 2: Load page data (here, just clear it for now)
            //page->clear();
            bool succ = loadPageFromDisk(pageId, *page);
            page->id = pageId;
            page->valid_bit = true;
            page->ref_count = 1;

            // Step 3: Insert into thePageMap
            thePageMap[pageId] = page;

            return *page;

        } else {
            std::cout << "read hit" << std::endl;
            Page* page = mapEntry->second;
            page->ref_count += 1;
            return *(page);
        }
    }

    bool PageCache::loadPageFromDisk(u32 pageId, Page& page) {
        off_t offset = pageId * PAGE_SIZE;
        ssize_t bytesRead = theDbFile.read_at(page.data, PAGE_SIZE, offset);
        return bytesRead == PAGE_SIZE;
    }
}