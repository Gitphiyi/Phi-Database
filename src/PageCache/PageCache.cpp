#include <unordered_map>
#include <stack>

#include "Page.hpp"
#include "PageCache.hpp"

PageCache::PageCache(u32 numPages)  : 
    BUFFER_SIZE(numPages * sizeof(Page)),
    NUM_PAGES(numPages)
{    
    thePages = new Page[NUM_PAGES];
    thePageMap.reserve(NUM_PAGES); //reserve pages to avoid rehashing
    
};
PageCache::~PageCache() {
    delete[] thePages;
}