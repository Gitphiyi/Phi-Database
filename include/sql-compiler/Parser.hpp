#pragma once

#include "general/Types.hpp"
#include "general/AST.hpp"
#include "sql-compiler/SqlAST.hpp"
#include "sql-compiler/Types.hpp"
#include <vector>
#include <unordered_map>

namespace DB {
    using queryFunc = void(*)(SqlNode*, std::vector<Token>&, int);

    // Query Rules
    //SELECT Statement grammer
    void    select_query(SqlNode* root, std::vector<Token>& tokens, int st); // all queries start with SELECT and end with ;, have a FROM denoting a single table, optional WHERE condition, and can have multiple selected columns
    void    select_expression_query(SqlNode* root, std::vector<Token>& tokens, int st);

    static std::unordered_map<string, queryFunc> start_tokens = {
        {"SELECT", &select_query}
    };

    std::vector<SqlNode>                 create_SQL_AST(std::vector<Token> tokens);
    std::vector<RANode>                     convert_to_RA(std::vector<SqlNode> sql_ast);    

//     <query> ::= "SELECT " <columns> " FROM " <name> <terminal> | "SELECT " <columns> " FROM " <name> " WHERE " <conditionList> <terminal>
// <columns> ::= (<name> ", ")+ | "*"
// <name> ::= <letter>+ | <letter>+ "_" | <letter>+ "_" <digit>+
// <conditionList> ::= <condition> <comparator> <condition>
// <comparator> ::= " AND " | " OR "
// <condition> ::= <name> <operator> <term>
// <operator> ::= " = " | " > " | " >= " | " < " | " <= "
// <term> ::= <digit> | <digit> "." <digit> | <name>
// <letter> ::= [a-z]+ | [A-Z]+
// <digit> ::= [1-9]+
// <terminal> ::= ";"


}