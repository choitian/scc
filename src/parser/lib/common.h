#pragma once

#include <Cstring>
#include <Cstdlib> 
#include <Cstdio> 
#include <Cassert>
#include <Cstdarg>

#include <string> 
#include <vector> 
#include <set> 
#include <list>
#include <queue>
#include <stack> 
#include <map> 
#include <algorithm>

#define IsDigit(c)         (c >= '0' && c <= '9')
#define IsOctDigit(c)      (c >= '0' && c <= '7')
#define ToUpper(c)		   (c & ~0x20)
#define IsHexDigit(c)      (IsDigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))
#define IsLetter(c)        ((c >= 'a' && c <= 'z') || (c == '_') || (c >= 'A' && c <= 'Z'))


std::string int_to_string(int i);
std::string int_int_to_string(int i,int j);

char* file_to_char_array(std::string file,int* size);
void string_to_file(std::string string,std::string file);
int DEBUG(const char * format, ...);

int skip_white_space(char* src,char* src_end,int single_line_comment,int block_comment,int continue_line,
	int* nskiped,int* nlines);
int one_line(char* src,char* src_end,int continue_line,int* size,int* nlines);
