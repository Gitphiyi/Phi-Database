#pragma once;

#include <cstddef>
#include <cstdlib>
#include <string>
#include <memory>

class Buffer {
    public:
    Buffer();
    ~Buffer();
    const int PAGE_SIZE;
    const int BUFFER_SIZE;
    const std::byte* buffer;

    int read();
    int write_through(); // write through
};