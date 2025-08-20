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
        //Identifier/Keyword
        if(isalpha(c)) {
            size_t start = i;
            char temp = query[i];
            while(i < query.length() && (isalpha(temp) || temp == '_')) {
                i++;
            }
            string str = query.substr(start, start - i);
            if(keywords.contains(str)) {
                tokens.push_back(Token(KEYWORD, str));
            } else {
                tokens.push_back(Token(IDENTIFIER, str));
            }
        }
        //set identifier for token
        else if(isnumber(c)) {
            size_t start = i;
            char temp = query[i];
            while(i < query.length() && isnumber(temp)) {
                i++;
            }
            tokens.push_back(Token(NUMBER, query.substr(start, i-start)));
        }
        //string literal
        else if(c == '\'') {
            i++;
            size_t start = i;
            while (i < input.size() && input[i] != '\'') i++;
            tokens.push_back({STRING, input.substr(start, i - start)});
            i++; //skip last apostrophe
        }
        // Operators
        else if (c == '=' || c == '>' || c == '<') {
            std::string op(1, c);
            if (i + 1 < input.size() && (input[i+1] == '=')) {
                op.push_back('=');
                i++;
            }
            tokens.push_back({OPERATOR, op});
            i++;
        }

        // Symbols
        else if (c == ',' || c == ';' || c == '(' || c == ')') {
            tokens.push_back({SYMBOL, std::string(1, c)});
            i++;
        }
        else {
            throw std::runtime_error("Unexpected character in SQL: " + std::string(1, c));
        }
    }
    return tokens;
}