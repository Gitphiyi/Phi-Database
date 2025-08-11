#include <unordered_map>
#include <stack>
#include <iostream>
#include <new>

#include "Page.hpp"
#include "DbFile.hpp"
#include "PageCache.hpp"

namespace DB {
    PageCache::PageCache(u32 numPages, DbFile& fileApi)  : 
        CACHE_SIZE(numPages * sizeof(Page)),
        NUM_PAGES(numPages),
        theDbFile(fileApi)
    {    
        thePageMap.reserve(NUM_PAGES); //reserve pages to avoid rehashing
    };

    bool PageCache::write_through(Page& page, const string& filepath) {
        int fd = theDbFile.add_path(filepath);
        ssize_t bytes_written = theDbFile.write_at(page.id, page, fd);
        std::cout << bytes_written << " bytes were written into " << filepath << std::endl;
        return true;
    }

    Page& PageCache::read(u32 pageId, Page& buffer, const string& filepath) {
        //check cache to see if page exists
        if(thePageMap.contains(pageId)) {
            buffer = *(thePageMap[pageId]);
            return buffer;
        }
        else {
            std::cout << "cache read miss" << std::endl;
        }

        //read page into buffer from disk
        int fd = theDbFile.get_path(filepath);
        if(fd == -1) {
            std::cout << "path is invalid" << std::endl;
        }
        //need to read from an address
        ssize_t bytes_read = theDbFile.read_at(pageId, buffer, fd);

        //evict_page(buffer, thePageMap, theUsedPages, NUM_PAGES);
        std::cout << bytes_read << " bytes were read from " << filepath << std::endl;


        return buffer;
    }

    void evict_page(Page& page, std::unordered_map<u32, Page*>& pageMap, std::stack<Page*>& usedPages, u64 numPages) {
        if(usedPages.size() < numPages) {
            usedPages.push(&page);
            pageMap[page.id] = &page;
        } //cache has space 
        else {
            Page* evicted_page = usedPages.top();
            usedPages.pop();
            pageMap.erase(evicted_page->id);
            pageMap[page.id] = &page;
        }

    }
    
    // Page& PageCache::read(u32 pageId, const string& filepath) {
    //     auto mapEntry = thePageMap.find(pageId);
    //     if(mapEntry == thePageMap.end() || !mapEntry->second->valid_bit) {
    //         std::cout << "read miss" << std::endl;

    //         //Step 1: get a free page
    //         Page* page = nullptr;
    //         //check if there are free pages
    //         if (!theFreePages.empty()) {
    //             page = theFreePages.top();
    //             theFreePages.pop();
    //         } else {
    //             if (thePageMap.empty()) {
    //                 throw std::runtime_error("Page cache is empty and cannot evict!");
    //             }
    //             auto victim = thePageMap.begin();
    //             page = victim->second;
    //             thePageMap.erase(victim);
    //             // Here, you might write back dirty pages, etc.
    //         }

    //         // Step 2: Load page data (here, just clear it for now)
    //         //page->clear();
    //         bool succ = loadPageFromDisk(pageId, *page);
    //         page->valid_bit = true;
    //         page->ref_count = 1;

    //         // Step 3: Insert into thePageMap
    //         thePageMap[pageId] = page;

    //         return *page;

    //     } else {
    //         std::cout << "read hit" << std::endl;
    //         Page* page = mapEntry->second;
    //         page->ref_count += 1;
    //         return *(page);
    //     }
    // }
}