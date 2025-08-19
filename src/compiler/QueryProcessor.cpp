#include "compiler/QueryProcessor.hpp"

#include <vector>
#include <algorithm>
#include <cctype>
#include <string>

using namespace DB;

std::vector<Token> tokenize_query(string& query) {
    std::vector<Token> tokens;

    //lower case query in place
    std::transform(query.begin(), query.end(), query.begin(),
    [](unsigned char c){ return std::tolower(c); }); 

    size_t i = 0;
    while(i < query.length()) {
        char c = query[i];
        if(isspace(c)) {
            i++;
            continue;
        }
        //check if it is keyword or not
        if(isalpha(c)) {

        }
        //set identifier for token
        if(isnumber(c)) {

        }
    }
    return tokens;
}