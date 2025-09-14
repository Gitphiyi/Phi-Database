#pragma once

#include "general/Types.hpp"
#include "general/AST.hpp"
#include "sql-compiler/SqlAST.hpp"
#include "sql-compiler/Types.hpp"
#include <vector>

namespace DB {
    std::unordered_set<string> start_tokens = {
        "SELECT", "INSERT", "UPDATE", "DELETE", "CREATE", "ALTER", "DROP", "WITH"
    };

    std::vector<SqlNode>                 create_SQL_AST(std::vector<Token> tokens);
    

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
    // query rules
    void    query(); // all queries start with SELECT and end with ;, have a FROM denoting a single table, optional WHERE condition, and can have multiple selected columns
    bool    columns();

}