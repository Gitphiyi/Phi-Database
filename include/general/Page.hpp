#pragma once

#include "Types.hpp"
#include <string>
#include <iostream>
#include <cstring>


#define PAGE_SIZE sizeof(Page) + PAGE_DATA_SIZE - sizeof(std::byte*) //4KB Page which replaces data array pointer with page size
#define PAGE_DATA_SIZE 128 // in bytes
#define PAGE_FILL 90 //make sure only fill up to 90%

struct Page {
    bool        valid_bit; //contains valid info (not empty, not junk, etc.)
    bool        dirty_bit; //is the stuff on the page dirty as in it has writes to it ?
    u16         ref_count; //how many tables reference this page or is it a database header
    u32         id;
    u32         used_bytes; //byte where stuff can get written in
    std::byte*  data[PAGE_DATA_SIZE];

    Page() : dirty_bit(false), valid_bit(false), ref_count(0), id(0), used_bytes(0) {}
    Page(u32 id) : dirty_bit(false), valid_bit(false), ref_count(0), id(id) {}
    Page operator=(Page& o) {
        this->valid_bit = o.valid_bit;
        this->dirty_bit = o.dirty_bit;
        this->ref_count = o.dirty_bit;
        this->id = o.id;
        this->used_bytes = o.used_bytes;
        std::memcpy(data, o.data, PAGE_DATA_SIZE);
        return *this;
    }

    void write_data(void* addedData, u32 sz) {
        void* ptr = data + used_bytes;
        std::memcpy(ptr, addedData, sz);
        used_bytes += sz;
    }

    void clear() { std::memset(data, 0, PAGE_DATA_SIZE); }

    template<typename T>
    void print() {
        std::cout << "----------Page----------" << std::endl;
        std::cout << "valid bit: " << valid_bit << std::endl;
        std::cout << "dirty bit: " << dirty_bit << std::endl;
        std::cout << "ref_count: " << ref_count << std::endl;
        std::cout << "used data: " << used_bytes << std::endl;
        std::cout << "page data: " << std::endl;

        if (PAGE_DATA_SIZE % sizeof(T) != 0) {
            perror("Type attempting to be printed is not aligned with page size");
            return;
        }
        for(int i = 0; i < PAGE_DATA_SIZE / sizeof(T); i ++){
            T num;
            char* ptr = reinterpret_cast<char*>(data) + i * sizeof(T);
            std::memcpy(&num, (data+i*sizeof(T)), sizeof(T));
            std::cout << num;
            if( i < (PAGE_DATA_SIZE / sizeof(T)) - 1) {
                std::cout << ", ";
            }
        }
        std::cout << std::endl;
        std::cout << "------------------------" << std::endl;
    }
};

//static_assert(sizeof(Page) == PAGE_SIZE,"Page must occupy exactly one on-disk page");