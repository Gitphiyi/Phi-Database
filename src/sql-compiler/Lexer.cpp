#include "sql-compiler/Lexer.hpp"
#include <iostream>

namespace DB {
    std::vector<Token> tokenize_query(string& query) {
        std::vector<Token> tokens;

        //lower case query in place
        std::transform(query.begin(), query.end(), query.begin(),
        [](unsigned char c){ return std::toupper(c); }); 

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
                while(i < query.length() && (isalpha(query[i]) || query[i] == '_')) { 
                    i++; 
                }
                string str = query.substr(start, i - start);
                if(keywords.contains(str)) {
                    tokens.push_back(Token(KEYWORD, str));
                } else {
                    tokens.push_back(Token(IDENTIFIER, str));
                }
                continue;
            }
            //set identifier for token
            if(isnumber(c)) {
                size_t start = i;
                while(i < query.length() && isnumber(query[i])) {
                    i++;
                }
                tokens.push_back(Token(NUMBER, query.substr(start, i-start)));
                continue;
            }
            //string literal
            if(c == '\'') {
                i++;
                size_t start = i;
                while (i < query.size() && query[i] != '\'') i++;
                tokens.push_back({STRING, query.substr(start, i - start)});
                i++; //skip last apostrophe
                continue;
            }
            // Symbols
            if (symbols.contains(string(1, c))) {
                tokens.push_back({SYMBOL, std::string(1, c)});
                i++;
                continue;
            }
            // Rest can only be operators or error
            if (sql_ops.contains(string(1, c)) || c == ':') {
                std::string op(1, c);
                if(c == ':' && i+1 < query.length() && query[i+1] == ':') {
                    tokens.push_back({OPERATOR, "::"});
                    i += 2;
                    continue;
                }
                if (i + 1 < query.size() && (query[i+1] == '=')) {
                    op.push_back('=');
                    i++;
                }
                tokens.push_back({OPERATOR, op});
                i++;
                continue;
            }
            error: 
            throw std::runtime_error("Unexpected character in SQL: " + string(1, c));
        }
        return tokens;
    }
}