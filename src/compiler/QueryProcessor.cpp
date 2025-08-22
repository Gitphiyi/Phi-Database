#include "compiler/QueryProcessor.hpp"

#include <iostream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <string>

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
                std::cout << "space \n";
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
                std::cout << str << std::endl;
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
            // Operators
            if (comparison_ops.contains(string(1, c))) {
                std::string op(1, c);
                if (i + 1 < query.size() && (query[i+1] == '=')) {
                    op.push_back('=');
                    i++;
                }
                tokens.push_back({OPERATOR, op});
                i++;
                continue;
            }
            // Symbols
            if (symbols.contains(string(1, c))) {
                tokens.push_back({SYMBOL, std::string(1, c)});
                i++;
                continue;
            }
            throw std::runtime_error("Unexpected character in SQL: " + string(1, c));
        }
        return tokens;
    }

    //creates AST from tokens
    void parse_syntax(std::vector<Token> tokens) {

    }

    //checks AST to make sure all table/cols exist, type checking, validate aggregates, etc.
    void analyze_semantics() {

    }

    //convert AST to relational algebra tree
    void convert_to_ra() {

    }

    // optionally optimize plan

    //hand off ra queries to table to be executed as table ops
}