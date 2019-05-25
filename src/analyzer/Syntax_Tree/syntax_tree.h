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
#include "..\lib\common.h"

struct syntax_node
{
	struct syntax_node* parent;
	syntax_node(){ parent=NULL;}
	std::string op;
	std::string pos;
	virtual bool is_terminal(std::string *tok,std::string *extra)
	{
		return false;
	}
	virtual bool is_null()
	{
		return false;
	}
//semantic
	virtual void check()
	{
		::SCC_ERROR(pos,"Not Defined Yet.\n");
	}
};
struct TERMINAL_node:public syntax_node
{
	std::string tok;
	std::string extra;
	TERMINAL_node(std::string tok,std::string extra)
	{
		op="TERMINAL";
		this->tok=tok;
		this->extra=extra;
	}
	virtual bool is_terminal(std::string *tok,std::string *extra)
	{
		if(tok!=NULL)
			*tok=this->tok;
		if(extra!=NULL)
			*extra=this->extra;
		return true;

	}
	virtual bool is_null()
	{
		return tok=="__NULL__";
	}	
};






