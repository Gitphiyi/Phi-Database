#include "include/PageCache.hpp"

PageCache::PageCache()  : 
    PAGE_SIZE   (8  * 1024),         //  8 KB
    BUFFER_SIZE(1000 * 1024),        // 1000 KB
    buffer     (nullptr)
{    
    buffer = new std::byte[BUFFER_SIZE];
};
PageCache::~PageCache() {
    delete[] buffer;
}