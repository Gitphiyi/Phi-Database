#pragma once;

#include <cstddef>
#include <cstdlib>
#include <string>
#include <memory>

class PageCache {
    public:
    PageCache();
    ~PageCache();
    const int PAGE_SIZE;
    const int BUFFER_SIZE;
    const std::byte* buffer;

    int read(); 
    int write_through(); // write through
};