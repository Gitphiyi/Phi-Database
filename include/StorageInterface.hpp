#pragma once

int create_file(const char* filename);
int write_file(const char* filename);
void* read_file(const char* filename);
int delete_file(const char* filename);
int close_file(const char* filename);