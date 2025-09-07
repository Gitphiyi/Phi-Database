#pragma once

#include "general/Types.hpp"
#include <exception>
namespace DB {
    inline CondFn condfn_generator(string cond) {
        if(cond == "="){
            return [=](datatype c, datatype o) { return c == o; };
        }
        else if (cond == "<>" || cond == "!=") {
            return [=](datatype c, datatype o) { return c != o; };
        }
        else if (cond == "<") {
            return [=](datatype c, datatype o) { return c < o; };
        }
        else if (cond == ">") {
            return [=](datatype c, datatype o) { return c > o; };
        }
        else if (cond == "<=") {
            return [=](datatype c, datatype o) { return c <= o; };
        }
        else if (cond == ">=") {
            return [=](datatype c, datatype o) { return c >= o; };
        }
        throw std::invalid_argument( "received condition that doesn't exist" );
        return nullptr;
    }
}
