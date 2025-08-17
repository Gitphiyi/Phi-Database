#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <variant>

using u64 = std::uint64_t;
using u32 = std::uint32_t;
using u16 = std::uint16_t;
using u8  = std::uint8_t;
using size_t = std::size_t;
using string = std::string;
using datatype = std::variant<int, float, std::string, bool, int64_t>;


enum Orientation {
    COLUMN,
    ROW
};