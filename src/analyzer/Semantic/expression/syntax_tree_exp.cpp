#include <Cstring>
#include <Cstdlib> 
#include <Cstdio> 
#include <Cassert>
#include <Cstdarg>
#include <Climits>

#include <string> 
#include <vector> 
#include <set> 
#include <list>
#include <queue>
#include <stack> 
#include <map>
#include <algorithm>

#include "..\..\lib\common.h"
#include "..\semantic_analyzer.h"
#include "..\..\Syntax_Tree\syntax_tree.h"
#include "..\..\Syntax_Tree\syntax_tree_decl.h"
#include "..\..\Syntax_Tree\syntax_tree_stmt.h"
#include "..\..\Syntax_Tree\syntax_tree_exp.h"
#include "..\Symbol_Table\type.h"
#include "..\Symbol_Table\identifier.h"
#include "..\Symbol_Table\scope.h"
#include "IC.h"


void exp::check_as_branch(struct IC_INS_BLOCK *iftrue,struct IC_INS_BLOCK *iffalse)
{
	struct exp_UNEQUAL* node=new exp_UNEQUAL;
	struct exp_CONSTANT_INTEGER *const_zero=new exp_CONSTANT_INTEGER();
	const_zero->lexical_value="0";
	node->operand_list.push_back(this);
	node->operand_list.push_back(const_zero);
	node->check_as_branch(iftrue,iffalse);
}
void exp::as_default()
{
	struct CONST *c=new CONST();
	c->type=TYPE::void_type();
	this->addr=c;
}
void exp::check_operands()
{
	for(size_t i=0;i<this->operand_list.size();i++)
	{
		struct exp* operand=this->operand_list.at(i);
		operand->check();
		operand->adjust(this->op,i==0);
	}
}
void exp::adjust(std::string op,bool left_operand)
{
	if(op=="exp_SIZE_OF_TYPE" || op=="exp_SIZE_OF_OBJ" || op=="exp_ADDR" ||
		op=="exp_PRE_INC" || op=="exp_PRE_DEC" || op=="exp_POST_INC" || op=="exp_POST_DEC" ||op=="exp_OBJ_MBSL" ||
		(left_operand && 
		( op=="exp_ASSIGN" ||
		op=="exp_ADD_ASSIGN" ||
		op=="exp_SUB_ASSIGN" ||
		op=="exp_MUL_ASSIGN" ||
		op=="exp_DIV_ASSIGN" ||
		op=="exp_MOD_ASSIGN" ||
		op=="exp_LSHIFT_ASSIGN" ||
		op=="exp_RSHIFT_ASSIGN" ||
		op=="exp_BITAND_ASSIGN" ||
		op=="exp_BITXOR_ASSIG" ||
		op=="exp_BITOR_ASSIGN" 
		)))
	{
	}else
	{
		if(this->addr->is_lvalue && !this->addr->type->is_array())
		{
			this->addr->is_lvalue=false;
			this->addr->type=this->addr->type->unqualify();
		}
	}
	if(op=="exp_SIZE_OF_TYPE" || op=="exp_SIZE_OF_OBJ" || op=="exp_ADDR")
	{
	}else
	{
		if(this->addr->is_lvalue && this->addr->type->is_array())
		{
			struct TYPE* type=TYPE::pointer_type(static_cast<struct ARRAY_TYPE*>(this->addr->type)->type_);
			this->addr=MEM::addr_of(this->addr);
			this->addr->type=type;
			this->addr->is_lvalue=false;
		}
	}
	if(op=="exp_SIZE_OF_TYPE" || op=="exp_SIZE_OF_OBJ" || op=="exp_ADDR")
	{
	}else
	{
		if(this->addr->type->is_function())
		{
			struct TYPE* type=TYPE::pointer_type(this->addr->type);
			this->addr=MEM::addr_of(this->addr);
			this->addr->type=type;
			this->addr->is_lvalue=false;
		}
	}
}





