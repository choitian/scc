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

std::string string_format(const char * format, ...);
std::string int_to_string(int i);
int string_to_int(std::string number);

char* file_to_char_array(std::string file,int* size);
void string_to_file(std::string string,std::string file);

int skip_white_space(char* src,char* src_end,int single_line_comment,int block_comment,int continue_line,
	int* nskiped,int* nlines);
int one_line(char* src,char* src_end,int continue_line,int* size,int* nlines);

int START_DEBUG(std::string file);
int SCC_DEBUG(const char * format, ...);
int SCC_MSG(const char * format, ...);
int SCC_ERROR(std::string pos,const char * format, ...);
int SCC_SYNTAX_ERROR(std::string pos,const char * format, ...);
int SCC_WARNNING(std::string pos,const char * format, ...);

size_t ALIGN(size_t size,size_t align);
struct ENVIRONMENT
{
	std::string DFA_table;
	std::string LALR_table;
	std::string input_file;
	std::string debug_file;
	FILE* debug_FP;
	std::string ic_file;
	std::string asm_file;

	int error_num;
	int warnning_num;

	ENVIRONMENT();
};
extern struct ENVIRONMENT SCC_ENV;