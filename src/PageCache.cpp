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
        theFreePages.reserve(NUM_PAGES);
    
        // fill cache with pages 
        theCachePages = new Page[numPages];
        for(int i = 0; i < numPages; i++) {
            theFreePages.push_back(i);
        }
    };

    void PageCache::evict_add_page(Page& page) {
        if(theUsedPages.size() < NUM_PAGES) {
            // place page into cache
            size_t idx = theFreePages[theFreePages.size()-1];

            theFreePages.pop_back();
            theCachePages[idx] = page;
            theUsedPages.push(idx);
            thePageMap[page.id] = idx;

        } //cache has space 
        else {
            size_t evicted_idx = theUsedPages.top();
            Page evicted_page = theCachePages[evicted_idx];
            theUsedPages.pop();
            thePageMap.erase(evicted_page.id);

            //place page into cache
            theCachePages[evicted_idx] = page;
            thePageMap[page.id] = evicted_idx;
            std::cout << "evicted page " << evicted_page.id << std::endl;
        } //cache is full
    }

    bool PageCache::write_through(Page& page, const string& filepath) {
        //write page to disk
        int fd = theDbFile.get_path(filepath);
        if(fd == -1) {
            fd = theDbFile.add_path(filepath);
        }
        ssize_t bytes_written = theDbFile.write_at(page.id, page, fd);
        std::cout << bytes_written << " bytes were written into " << filepath << std::endl;
        
        //add page to cache
        evict_add_page(page);
        return true;
    }

    Page& PageCache::read(u32 pageId, Page& buffer, const string& filepath) {
        //check cache to see if page exists
        if(thePageMap.contains(pageId)) {
            std::cout << "cache read hit" << std::endl;
            size_t idx = thePageMap[pageId];
            buffer = theCachePages[idx];
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

        evict_add_page(buffer);
        std::cout << bytes_read << " bytes were read from " << filepath << std::endl;


        return buffer;
    }

    void PageCache::print() {
        std::cout << "----------PageCache----------" << std::endl;
        std::cout << "Cache Size: " << CACHE_SIZE << " bytes" << std::endl;
        std::cout << "Number of Pages: " << NUM_PAGES << std::endl;

        std::cout << "PageCache map (id => page address): \n{" << std::endl;
        for(auto it = thePageMap.begin(); it != thePageMap.end(); ++it) {
            size_t idx = it->second;
            std::cout << "\t" << it->first << " => idx " << idx << " has page id " << theCachePages[idx].id << ",\n";
        }
        std::cout << "}" << std::endl;
        std::cout << "-----------------------------" << std::endl;
    }
    
}