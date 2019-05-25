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
namespace
{
	bool equalable_pointer(struct ADDRESS* operand0,struct ADDRESS* operand1)
	{
		if(operand0->type->is_pointer() && operand1->type->is_pointer())
		{
			struct POINTER_TYPE* pointer0=static_cast<struct POINTER_TYPE*>(operand0->type);
			struct POINTER_TYPE* pointer1=static_cast<struct POINTER_TYPE*>(operand1->type);
			if(pointer0->type_->is_compatible(pointer1->type_))
			{
				return true;
			}
			if((pointer0->type_->is_void() && (pointer1->type_->is_object_type() || pointer1->type_->is_incomplete())) ||
				(pointer1->type_->is_void() && (pointer0->type_->is_object_type() || pointer0->type_->is_incomplete())))
			{
				return true;
			}
		}else if( (operand0->type->is_pointer() && operand1->is_CONST() && operand1->type->is_integral_type() && static_cast<struct CONST*>(operand1)->is_zero()) ||
			(operand1->type->is_pointer() && operand0->is_CONST() && operand0->type->is_integral_type() && static_cast<struct CONST*>(operand0)->is_zero()) )
		{
				return true;
		}
		return false;
	}
	bool comparable_pointer(struct ADDRESS* operand0,struct ADDRESS* operand1)
	{
		if(operand0->type->is_pointer() && operand1->type->is_pointer())
		{
			struct POINTER_TYPE* pointer0=static_cast<struct POINTER_TYPE*>(operand0->type);
			struct POINTER_TYPE* pointer1=static_cast<struct POINTER_TYPE*>(operand1->type);
			if(pointer0->type_->is_compatible(pointer1->type_))
			{
				if(pointer0->type_->is_object_type() && pointer1->type_->is_object_type())
					return true;
				if(pointer0->type_->is_incomplete() && pointer1->type_->is_incomplete())
					return true;
			}
		}
		return false;
	}
	void logic_check_as_branch(struct exp* expr,bool (pointer_constraint)(struct ADDRESS*,struct ADDRESS*),struct IC_INS_BLOCK *iftrue,struct IC_INS_BLOCK *iffalse)
	{
		expr->check_operands();
		struct ADDRESS* operand0=expr->operand_list[0]->addr;
		struct ADDRESS* operand1=expr->operand_list[1]->addr;

		if(operand0->type->is_arithmetic_type() && operand1->type->is_arithmetic_type())
		{
			struct TYPE* type=usual_arithmetic_conversion(operand0->type,operand1->type);
			operand0=IC_CVT::cast_to(operand0,type);
			operand1=IC_CVT::cast_to(operand1,type);
			IJUMP::new_(expr->lex_op,operand0,operand1,iftrue,iffalse);
		}else if(pointer_constraint(operand0,operand1))
		{
			IJUMP::new_(expr->lex_op,operand0,operand1,iftrue,iffalse);
		}
		else
		{
			::SCC_ERROR(expr->pos,"%d : uncomparable type.",expr->lex_op.c_str());
		}
	}
	void logic_check_as_value(struct exp* expr,bool (pointer_constraint)(struct ADDRESS*,struct ADDRESS*))
	{
		expr->check_operands();
		struct ADDRESS* operand0=expr->operand_list[0]->addr;
		struct ADDRESS* operand1=expr->operand_list[1]->addr;

		if(operand0->type->is_arithmetic_type() && operand1->type->is_arithmetic_type())
		{
			expr->addr=ISET::new_(expr->lex_op,operand0,operand1);
		}else if(pointer_constraint(operand0,operand1))
		{
			struct TYPE* type0=operand0->type;
			struct TYPE* type1=operand1->type;

			operand0->type=TYPE::basic_type("uint32");
			operand1->type=TYPE::basic_type("uint32");
			expr->addr=ISET::new_(expr->lex_op,operand0,operand1);
			operand0->type=type0;
			operand1->type=type1;
		}
		else
		{
			::SCC_ERROR(expr->pos,"%d : uncomparable type.",expr->lex_op.c_str());
			expr->as_default();
		}
	}
}
void exp_RELATIONAL::check_as_branch(struct IC_INS_BLOCK *iftrue,struct IC_INS_BLOCK *iffalse)
{
	if(this->op=="exp_EQUAL" || this->op=="exp_UNEQUAL" )
		logic_check_as_branch(this,equalable_pointer,iftrue,iffalse);
	else
		logic_check_as_branch(this,comparable_pointer,iftrue,iffalse);
}
void exp_RELATIONAL::check()
{
	if(this->op=="exp_EQUAL" || this->op=="exp_UNEQUAL" )
		logic_check_as_value(this,equalable_pointer);
	else
		logic_check_as_value(this,comparable_pointer);
}

void exp_AND::check_as_branch(struct IC_INS_BLOCK *iftrue,struct IC_INS_BLOCK *iffalse)
{
/*
fst.check(NULL,iffalse)
snd.check(iftrue,iffalse)
*/
	struct exp* operand_exp0=this->operand_list[0];
	struct exp* operand_exp1=this->operand_list[1];
	if(iftrue!=NULL)
	{
		struct IC_INS_BLOCK *next=IC_INS_BLOCK::new_();
		operand_exp0->check_as_branch(NULL,next);
		operand_exp1->check_as_branch(iftrue,NULL);
		next->start();
	}else if(iffalse!=NULL)
	{
		operand_exp0->check_as_branch(NULL,iffalse);
		operand_exp1->check_as_branch(iftrue,iffalse);
	}
}
void exp_OR::check_as_branch(struct IC_INS_BLOCK *iftrue,struct IC_INS_BLOCK *iffalse)
{
/*
fst.check(iftrue,NULL)
snd.check(iftrue,iffalse)
*/
	struct exp* operand_exp0=this->operand_list[0];
	struct exp* operand_exp1=this->operand_list[1];
	if(iffalse!=NULL)
	{
		struct IC_INS_BLOCK *next=IC_INS_BLOCK::new_();
		operand_exp0->check_as_branch(next,NULL);
		operand_exp1->check_as_branch(iftrue,iffalse);
		next->start();
	}else if(iftrue!=NULL)
	{
		operand_exp0->check_as_branch(iftrue,NULL);
		operand_exp1->check_as_branch(iftrue,iffalse);
	}
}
void exp_NOT::check_as_branch(struct IC_INS_BLOCK *iftrue,struct IC_INS_BLOCK *iffalse)
{
/*
fst.check(iffalse,iftrue)
*/
	struct exp* operand_exp0=this->operand_list[0];
	operand_exp0->check_as_branch(iffalse,iftrue);
}
bool exp_AND::can_constant_fold()
{
	struct exp* operand_exp0=this->operand_list[0];
	struct exp* operand_exp1=this->operand_list[1];
	bool discard_ins_old=iFUNCTION::current_definition->discard_ins;
	iFUNCTION::current_definition->discard_ins=true;
	operand_exp0->check();
	operand_exp1->check();
	iFUNCTION::current_definition->discard_ins=discard_ins_old;
	if(operand_exp0->addr->is_CONST() && operand_exp0->addr->is_CONST() && operand_exp1->addr->type->is_arithmetic_type()&&
		operand_exp1->addr->is_CONST())
	{
		if(static_cast<struct CONST*>(operand_exp0->addr)->is_zero() || static_cast<struct CONST*>(operand_exp1->addr)->is_zero())
		{
			this->addr=new CONST(0);
		}else
			this->addr=new CONST(1);
		return true;
	}
	return false;
}
void exp_AND::check()
{
/*
fst.check(NULL,setfalse)
snd.check(NULL,setfalse);
t=1;
jump next
setfalse:
t=0;
next:
*/
	if(can_constant_fold())
		return;
	struct IC_INS_BLOCK *next=IC_INS_BLOCK::new_();
	struct IC_INS_BLOCK *setfalse=IC_INS_BLOCK::new_();
	struct exp* operand_exp0=this->operand_list[0];
	struct exp* operand_exp1=this->operand_list[1];
	struct ADDRESS* result=MEM::local_temp(TYPE::basic_type("int32"));
	operand_exp0->check_as_branch(NULL,setfalse);
	operand_exp1->check_as_branch(NULL,setfalse);
	IC_MOVE::new_(result,new CONST(1));
	JUMP::new_(next);
	setfalse->start();
	IC_MOVE::new_(result,new CONST(0));
	next->start();
	this->addr=result;
}
bool exp_OR::can_constant_fold()
{
	struct exp* operand_exp0=this->operand_list[0];
	struct exp* operand_exp1=this->operand_list[1];
	bool discard_ins_old=iFUNCTION::current_definition->discard_ins;
	iFUNCTION::current_definition->discard_ins=true;
	operand_exp0->check();
	operand_exp1->check();
	iFUNCTION::current_definition->discard_ins=discard_ins_old;
	if(operand_exp0->addr->is_CONST() && operand_exp0->addr->is_CONST() && operand_exp1->addr->type->is_arithmetic_type()&&
		operand_exp1->addr->is_CONST())
	{
		if(static_cast<struct CONST*>(operand_exp0->addr)->is_zero() && static_cast<struct CONST*>(operand_exp1->addr)->is_zero())
		{
			this->addr=new CONST(0);
		}else
			this->addr=new CONST(1);
		return true;
	}
	return false;
}
void exp_OR::check()
{
/*
fst.check(settrue,NULL)
snd.check(settrue,NULL);
t=0;
jump next
settrue:
t=1;
next:
*/
	if(can_constant_fold())
		return;
	struct IC_INS_BLOCK *next=IC_INS_BLOCK::new_();
	struct IC_INS_BLOCK *settrue=IC_INS_BLOCK::new_();
	struct exp* operand_exp0=this->operand_list[0];
	struct exp* operand_exp1=this->operand_list[1];
	struct ADDRESS* result=MEM::local_temp(TYPE::basic_type("int32"));
	operand_exp0->check_as_branch(settrue,NULL);
	operand_exp1->check_as_branch(settrue,NULL);
	IC_MOVE::new_(result,new CONST(0));
	JUMP::new_(next);
	settrue->start();
	IC_MOVE::new_(result,new CONST(1));
	next->start();
	this->addr=result;
}
void exp_NOT::check()
{
	struct exp_EQUAL* node=new exp_EQUAL;
	struct exp_CONSTANT_INTEGER *const_zero=new exp_CONSTANT_INTEGER();
	const_zero->lexical_value="0";
	node->operand_list.push_back(this->operand_list[0]);
	node->operand_list.push_back(const_zero);
	node->check();

	this->addr=node->addr;
}

