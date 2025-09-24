#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <variant>
#include <functional>

using u64 = std::uint64_t;
using u32 = std::uint32_t;
using u16 = std::uint16_t;
using u8  = std::uint8_t;
using size_t = std::size_t;
using string = std::string;
using datatype = std::variant<int, float, string, bool, int64_t, double>;
using CondFn = std::function<bool(datatype, datatype)>;


enum Orientation {
    COLUMN,
    ROW
};

enum OpType {
    READ,
    WRITE,
    CREATE,
    LOCK,
    UNLOCK,
    COMMIT
};

enum class ColumnType {
    INT,
    FLOAT,
    CHAR,
    BOOL,
    INT64,
    DOUBLE,
    STRING
};