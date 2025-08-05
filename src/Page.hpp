#pragma once
#include "Types.hpp"
#include <string>
#include <iostream>


#define PAGE_SIZE 4096 //4KB Page
#define PAGE_METADATA sizeof(bool) * 2 + sizeof(u16) * 2

struct Page {
    bool        valid_bit; //contains valid info (not empty, not junk, etc.)
    bool        dirty_bit;
    u16         ref_count;
    u16         free_idx; //array index for next free byte          
    std::byte   data[PAGE_SIZE]; 
    Page() : dirty_bit(false), valid_bit(false), ref_count(0), free_idx(0) {   
    }

    void clear() { std::memset(data, 0, PAGE_SIZE); }
    void print_bytes() {
        for(int i = 0; i < PAGE_SIZE; i++) {
            if( i == PAGE_SIZE - 1) {
                std::cout << std::endl;
            }
            std::cout << std::to_integer<unsigned char>(data[i]) << ", ";
        }
    }

    void print_int() {
        for(int i = 0; i < PAGE_SIZE; i+= sizeof(int)) {
            if( i == PAGE_SIZE - 1) {
                std::cout << std::endl;
            }
            int num;
            memcpy(&num, &data[i], sizeof(int));
            std::cout << num << ", ";
        }
    }

    std::byte* metadata_to_bytes() {
        std::byte* byte_arr = new std::byte[PAGE_METADATA];
        std::byte* p = byte_arr;
        memcpy(p, &valid_bit, sizeof(valid_bit));
        p += sizeof(valid_bit);
        memcpy(p, &dirty_bit, sizeof(dirty_bit));
        p += sizeof(dirty_bit);
        memcpy(p, &ref_count, sizeof(ref_count));
        p += sizeof(ref_count);
        memcpy(p, &free_idx, sizeof(free_idx));
        //OPTIMIZE THIS COPY LATER

        return byte_arr;
    }
};