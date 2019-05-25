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

struct exp_BITOR:public exp
{
	exp_BITOR()
	{
		op="exp_BITOR";
		lex_op="|";
	}
//sematic
	void check();
	static struct ADDRESS* check_operator(std::string pos,struct ADDRESS* operand0,struct ADDRESS* operand1);
};
struct exp_BITXOR:public exp
{
	exp_BITXOR()
	{
		op="exp_BITXOR";
		lex_op="^";
	}
//sematic
	void check();
	static struct ADDRESS* check_operator(std::string pos,struct ADDRESS* operand0,struct ADDRESS* operand1);
};
struct exp_BITAND:public exp
{
	exp_BITAND()
	{
		op="exp_BITAND";
		lex_op="&";
	}
//sematic
	void check();
	static struct ADDRESS* check_operator(std::string pos,struct ADDRESS* operand0,struct ADDRESS* operand1);
};
struct exp_RSHIFT:public exp
{
	exp_RSHIFT()
	{
		op="exp_RSHIFT";
		lex_op=">>";
	}
//sematic
	void check();
	static struct ADDRESS* check_operator(std::string pos,struct ADDRESS* operand0,struct ADDRESS* operand1);
};
struct exp_LSHIFT:public exp
{
	exp_LSHIFT()
	{
		op="exp_LSHIFT";
		lex_op="<<";
	}
//sematic
	void check();
	static struct ADDRESS* check_operator(std::string pos,struct ADDRESS* operand0,struct ADDRESS* operand1);
};

struct exp_POS:public exp
{
	exp_POS()
	{
		op="exp_POS";
		lex_op="+";
	}
//sematic
	void check();
};
struct exp_COMP:public exp
{
exp_COMP()
{
	op="exp_COMP";
	lex_op="~";
}
//sematic
	void check();
};


struct exp_NEG:public exp
{
	exp_NEG()
	{
		op="exp_NEG";
		lex_op="-";
	}
//sematic
	void check();
};
struct exp_ADD:public exp
{
exp_ADD()
{
	op="exp_ADD";
	lex_op="+";
}
//sematic
	void check();
	static struct ADDRESS* check_operator(std::string pos,struct ADDRESS* operand0,struct ADDRESS* operand1);
};
struct exp_SUB:public exp
{
exp_SUB()
{
	op="exp_SUB";
	lex_op="-";
}
//sematic
	void check();
	static struct ADDRESS* check_operator(std::string pos,struct ADDRESS* operand0,struct ADDRESS* operand1);
};
struct exp_MUL:public exp
{
exp_MUL()
{
	op="exp_MUL";
	lex_op="*";
}
//sematic
	void check();
	static struct ADDRESS* check_operator(std::string pos,struct ADDRESS* operand0,struct ADDRESS* operand1);
};
struct exp_DIV:public exp
{
exp_DIV()
{
	op="exp_DIV";
	lex_op="/";
}
//sematic
	void check();
	static struct ADDRESS* check_operator(std::string pos,struct ADDRESS* operand0,struct ADDRESS* operand1);
};
struct exp_MOD:public exp
{
exp_MOD()
{
	op="exp_MOD";
	lex_op="%";
}
//sematic
	void check();
	static struct ADDRESS* check_operator(std::string pos,struct ADDRESS* operand0,struct ADDRESS* operand1);
};






