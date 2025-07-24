#pragma once

#include <cstddef>
#include <cstdlib>
#include <string>
#include <memory>

class PageCache {
    public:
    PageCache();
    ~PageCache();
    int PAGE_SIZE;
    int BUFFER_SIZE;
    std::byte* buffer;

    int read(); 
    int write_through(); // write through
};