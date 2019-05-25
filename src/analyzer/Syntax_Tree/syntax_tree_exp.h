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

struct exp:public syntax_node
{
	std::string lex_op;
	std::vector<struct exp*> operand_list;
	void add_operand(struct syntax_node* operand)
	{
		operand->parent=this;
		operand_list.push_back(static_cast<struct exp*>(operand));
	}
//semantic
	struct ADDRESS* addr;
	void check_operands();
	virtual void check_as_branch(struct IC_INS_BLOCK *iftrue,struct IC_INS_BLOCK *iffalse);
	void adjust(std::string op,bool left_operand);
	void as_default();
};

struct exp_COMMA:public exp
{
	exp_COMMA()
	{
		op="exp_COMMA";
		lex_op=",";
	}
//sematic
	void check();
};
struct exp_CONDI:public exp
{
	exp_CONDI(){ op="exp_CONDI";}
//sematic
	void check();
	struct ADDRESS* options_check(struct IC_INS_BLOCK *true_adjust,struct IC_INS_BLOCK *false_adjust,struct ADDRESS* true_addr,struct ADDRESS* false_addr);
	bool can_constant_fold();
};
struct exp_CALL:public exp
{
	struct exp* designator;
	exp_CALL(){ op="exp_CALL";}
//sematic
	void check();
	struct ADDRESS* check_argument(size_t argument_index,struct TYPE* parameter_type,struct ADDRESS* argument);
};
struct exp_SIZE_OF_OBJ:public exp
{
	exp_SIZE_OF_OBJ()
	{
		op="exp_SIZE_OF_OBJ";
		lex_op="sizeof";
	}
//sematic
	void check();
};
struct exp_SIZE_OF_TYPE:public exp
{
	struct TYPE_NAME_DECLARATION* type_name;
	exp_SIZE_OF_TYPE()
	{
		op="exp_SIZE_OF_TYPE";
		lex_op="sizeof";
	}
//sematic
	void check();
};

#include "syntax_tree_exp_assignment.h"
#include "syntax_tree_exp_address.h"
#include "syntax_tree_exp_arithmetic.h"
#include "syntax_tree_exp_logic.h"






