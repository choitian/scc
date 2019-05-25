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
struct exp_ASSIGN:public exp
{
exp_ASSIGN()
{
	op="exp_ASSIGN";
	lex_op="=";
}
//sematic
	void check();
	static bool can_assign(struct TYPE* type,struct ADDRESS* value);
	static struct ADDRESS* check_operator(std::string pos,struct ADDRESS* operand0,struct ADDRESS* operand1);
};

struct exp_ADD_ASSIGN:public exp
{
	exp_ADD_ASSIGN()
	{
		op="exp_ADD_ASSIGN";
		lex_op="+=";
	}
//sematic
	void check();
};
struct exp_SUB_ASSIGN:public exp
{
	exp_SUB_ASSIGN()
	{
		op="exp_SUB_ASSIGN";
		lex_op="-=";
	}
//sematic
	void check();
};

struct exp_MUL_ASSIGN:public exp
{
	exp_MUL_ASSIGN()
	{
		op="exp_MUL_ASSIGN";
		lex_op="*=";
	}
//sematic
	void check();
};
struct exp_DIV_ASSIGN:public exp
{
	exp_DIV_ASSIGN()
	{
		op="exp_DIV_ASSIGN";
		lex_op="/=";
	}
//sematic
	void check();
};
struct exp_MOD_ASSIGN:public exp
{
	exp_MOD_ASSIGN()
	{
		op="exp_MOD_ASSIGN";
		lex_op="%=";
	}
//sematic
	void check();
};

struct exp_LSHIFT_ASSIGN:public exp
{
	exp_LSHIFT_ASSIGN()
	{
		op="exp_LSHIFT_ASSIGN";
		lex_op="<<=";
	}
//sematic
	void check();
};
struct exp_RSHIFT_ASSIGN:public exp
{
	exp_RSHIFT_ASSIGN()
	{
		op="exp_RSHIFT_ASSIGN";
		lex_op=">>=";
	}
//sematic
	void check();
};
struct exp_BITAND_ASSIGN:public exp
{
	exp_BITAND_ASSIGN()
	{
		op="exp_BITAND_ASSIGN";
		lex_op="&=";
	}
//sematic
	void check();
};
struct exp_BITXOR_ASSIGN:public exp
{
	exp_BITXOR_ASSIGN()
	{
		op="exp_BITXOR_ASSIG";
		lex_op="^=";
	}
//sematic
	void check();
};
struct exp_BITOR_ASSIGN:public exp
{
	exp_BITOR_ASSIGN()
	{
		op="exp_BITOR_ASSIGN";
		lex_op="|=";
	}
//sematic
	void check();
};

struct exp_PRE_INC:public exp
{
	exp_PRE_INC()
	{
		op="exp_PRE_INC";
		lex_op="++";
	}
//sematic
	void check();
};
struct exp_PRE_DEC:public exp
{
	exp_PRE_DEC()
	{
		op="exp_PRE_DEC";
		lex_op="--";
	}
//sematic
	void check();
};
struct exp_POST_INC:public exp
{
	exp_POST_INC()
	{
		op="exp_POST_INC";
		lex_op="++";
	}
//sematic
	void check();
};
struct exp_POST_DEC:public exp
{
	exp_POST_DEC()
	{
		op="exp_POST_DEC";
		lex_op="--";
	}
//sematic
	void check();
};






