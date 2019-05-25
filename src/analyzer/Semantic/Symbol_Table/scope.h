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

struct SCOPE
{
	virtual std::vector<struct IDENTIFIER*>* symbol_list()=0;

	virtual struct IDENTIFIER* find(std::string name)= 0;
	virtual struct IDENTIFIER* scope_find(std::string name)= 0;
	virtual void insert(std::string name,struct IDENTIFIER* identifier)= 0;
	virtual void clone_symbol_list(std::vector<struct IDENTIFIER*> *list)=0;

	virtual struct TYPE* find_tag(std::string name)= 0;
	virtual struct TYPE* scope_find_tag(std::string name)= 0;
	virtual void insert_tag(std::string name,struct TYPE* tag)= 0;

	virtual void enter_scope()=0;
	virtual void as_current()=0;
	virtual void exit_scope()=0;

	virtual bool is_file_scope()=0;

	static void start_file_scope();
	static struct SCOPE* file;
	static struct SCOPE* current;
};

