#pragma once
#include "Types.hpp"
#include <string>

#define PAGE_SIZE 4096 //4KB Page

struct Page {
    u32         id;
    bool        dirty_bit;
    bool        valid_bit; //contains valid info (not empty, not junk, etc.)
    u16         ref_count;
    u16         free_idx; //array index for next free byte          
    std::byte   data[PAGE_SIZE]; 
    Page(u32 id) : id(id), dirty_bit(false), valid_bit(false), ref_count(0), free_idx(0) {}

    void clear() { std::memset(data, 0, PAGE_SIZE); }
};