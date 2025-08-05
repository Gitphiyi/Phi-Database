#pragma once

#include "Types.hpp"
#include <string>
#include <iostream>


#define PAGE_SIZE 4096 //4KB Page
#define PAGE_METADATA sizeof(bool) * 2 + sizeof(u16)

struct Page {
    bool        valid_bit; //contains valid info (not empty, not junk, etc.)
    bool        dirty_bit;
    u16         ref_count;
    std::byte   data[PAGE_SIZE]; 
    Page() : dirty_bit(false), valid_bit(false), ref_count(0) {}

    void clear() { std::memset(data, 0, PAGE_SIZE); }

    template<typename T>
    void print() {
        std::cout << "valid bit: " << valid_bit << std::endl;
        std::cout << "dirty bit: " << dirty_bit << std::endl;
        std::cout << "ref_count: " << ref_count << std::endl;
        std::cout << "page data: " << std::endl;

        if (PAGE_SIZE % sizeof(T) != 0) {
            perror("Type attempting to be printed is not aligned with page size");
            return;
        }
        for(int i = 0; i < PAGE_SIZE; i+= sizeof(T)) {
            if( i == PAGE_SIZE - 1) {
                std::cout << std::endl;
            }
            int num;
            memcpy(&num, &data[i], sizeof(T));
            std::cout << num << ", ";
        }
    }
};