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

#include "syntax_tree.h"

struct exp_ADDR:public exp
{
	exp_ADDR()
	{
		op="exp_ADDR";
		lex_op="&";
	}
//sematic
	void check();
};
struct exp_DEREF:public exp
{
	exp_DEREF()
	{
		op="exp_DEREF";
		lex_op="&";
	}
//sematic
	void check();
};
struct exp_INDEX:public exp
{
	exp_INDEX()
	{
		op="exp_INDEX";
		lex_op="[]";
	}
//sematic
	void check();
};
struct exp_OBJ_MBSL:public exp
{
	std::string  member;
	exp_OBJ_MBSL()
	{
		op="exp_OBJ_MBSL";
		lex_op=".";
	}
//sematic
	void check();
};
struct exp_PTR_MBSL:public exp
{
	std::string  member;
	exp_PTR_MBSL()
	{
		op="exp_PTR_MBSL";
		lex_op="->";
	}
//sematic
	void check();
};
struct exp_CAST:public exp
{
	struct TYPE_NAME_DECLARATION* type_name;
	struct exp* expr;
	exp_CAST(){ op="exp_CAST";}
//sematic
	void check();
};

struct exp_ID:public exp
{
	std::string lexical_value;
	exp_ID(){ op="exp_ID";}
//sematic
	void check();
};
struct exp_CONSTANT_INTEGER:public exp
{
	std::string lexical_value;
	exp_CONSTANT_INTEGER(){ op="exp_CONSTANT_INTEGER";}
//sematic
	void check();
};
struct exp_CONSTANT_CHARACTER:public exp
{
	std::string lexical_value;
	exp_CONSTANT_CHARACTER(){ op="exp_CONSTANT_CHARACTER";}
//sematic
	void check();
};
struct exp_CONSTANT_FLOATING:public exp
{
	std::string lexical_value;
	exp_CONSTANT_FLOATING(){ op="exp_CONSTANT_FLOATING";}
//sematic
	void check();
};
struct exp_STRING:public exp
{
	std::string lexical_value;
	exp_STRING(){ op="exp_STRING";}
//sematic
	void check();
};







