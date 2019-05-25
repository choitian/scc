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


struct exp_NOT:public exp
{
	exp_NOT()
	{
		op="exp_NOT";
		lex_op="!";
	}
//sematic
	void check_as_branch(struct IC_INS_BLOCK *iftrue,struct IC_INS_BLOCK *iffalse);
	void check();
};
struct exp_OR:public exp
{
	exp_OR()
	{
		op="exp_OR";
		lex_op="||";
	}
//sematic
	bool can_constant_fold();
	void check_as_branch(struct IC_INS_BLOCK *iftrue,struct IC_INS_BLOCK *iffalse);
	void check();
};
struct exp_AND:public exp
{
	exp_AND()
	{
		op="exp_AND";
		lex_op="&&";
	}
//sematic
	bool can_constant_fold();
	void check_as_branch(struct IC_INS_BLOCK *iftrue,struct IC_INS_BLOCK *iffalse);
	void check();
};

struct exp_RELATIONAL:public exp
{
	void check_as_branch(struct IC_INS_BLOCK *iftrue,struct IC_INS_BLOCK *iffalse);
	void check();
};
struct exp_EQUAL:public exp_RELATIONAL
{
	exp_EQUAL()
	{
		op="exp_EQUAL";
		lex_op="==";
	}
};
struct exp_UNEQUAL:public exp_RELATIONAL
{
	exp_UNEQUAL()
	{
		op="exp_UNEQUAL";
		lex_op="!=";
	}
};
struct exp_LESS:public exp_RELATIONAL
{
	exp_LESS()
	{
		op="exp_LESS";
		lex_op="<";
	}
};
struct exp_GREAT_EQ:public exp_RELATIONAL
{
	exp_GREAT_EQ()
	{
		op="exp_GREAT_EQ";
		lex_op=">=";
	}
};
struct exp_GREAT:public exp_RELATIONAL
{
	exp_GREAT()
	{
		op="exp_GREAT";
		lex_op=">";
	}
};
struct exp_LESS_EQ:public exp_RELATIONAL
{
	exp_LESS_EQ()
	{
		op="exp_LESS_EQ";
		lex_op="<=";
	}
};


