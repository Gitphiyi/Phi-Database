#pragma once

#include "Table.hpp"

int create_table(Table& table);
int del_table(Table& table);
int add_table_col();
int del_table();
int drop_table();
/**
 * Remove all rows
 */
int truncate_table();
int rename_table();