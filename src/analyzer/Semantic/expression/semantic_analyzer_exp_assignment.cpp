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
	bool is_modifiable_lvalue(struct ADDRESS* addr)
	{
		if(!addr->is_lvalue)
			return false;
		if(addr->type->is_incomplete())
			return false;
		if(addr->type->is_const())
			return false;
		if(addr->type->is_struct() && static_cast<struct STRUCT_TYPE*>(addr->type->unqualify())->with_const_member_)
			return false;
		if(addr->type->is_union() && static_cast<struct UNION_TYPE*>(addr->type->unqualify())->with_const_member_)
			return false;
		return true;
	}
	bool is_qualifier_included(struct TYPE* type0,struct TYPE* type1)
	{
		if(type1->is_const() && !type0->is_const())
			return false;
		if(type1->is_volatile() && !type0->is_volatile())
			return false;
		return true;
	}
}
void exp_ASSIGN::check()
{
	this->check_operands();
	struct ADDRESS* operand0=this->operand_list[0]->addr;
	struct ADDRESS* operand1=this->operand_list[1]->addr;

	struct ADDRESS* t=exp_ASSIGN::check_operator(pos,operand0,operand1);
	if(t==NULL)
	{
		this->as_default();
	}else
	{
		this->addr=t;
	}

}
bool exp_ASSIGN::can_assign(struct TYPE* type,struct ADDRESS* value)
{
	if(type->is_arithmetic_type() && value->type->is_arithmetic_type())
	{
		return true;
	}else if(is_compatible_struct_or_union(type,value->type))
	{
		return true;
	}else if(type->is_pointer() && value->type->is_pointer())
	{
		struct POINTER_TYPE* pointer0=static_cast<struct POINTER_TYPE*>(type);
		struct POINTER_TYPE* pointer1=static_cast<struct POINTER_TYPE*>(value->type);
		if(pointer0->type_->is_compatible(pointer1->type_))
		{
			return true;
		}else if((pointer0->type_->is_void() && (pointer1->type_->is_object_type() || pointer1->type_->is_incomplete())) ||
			(pointer1->type_->is_void() && (pointer0->type_->is_object_type() || pointer0->type_->is_incomplete())))
		{
			return true;
		}
	}else if(type->is_pointer() && value->is_CONST() && value->type->is_integral_type() && static_cast<struct CONST*>(value)->is_zero())
	{
		return true;
	}
	return false;
}
struct ADDRESS* exp_ASSIGN::check_operator(std::string pos,struct ADDRESS* operand0,struct ADDRESS* operand1)
{
	struct ADDRESS* result;
	if(!is_modifiable_lvalue(operand0))
	{
		::SCC_ERROR(pos,"=: left operand need to be modifiable lvalue.");
	}
	if(operand0->type->is_arithmetic_type() && operand1->type->is_arithmetic_type())
	{
		operand1=IC_CVT::cast_to(operand1,operand0->type->unqualify());
		result=IC_MOVE::new_(operand0,operand1);
		result->type=operand0->type->unqualify();
		result->is_lvalue=false;
	}else if(is_compatible_struct_or_union(operand0->type,operand1->type))
	{
		result=operand0;
		result->type=operand0->type->unqualify();
		IC_MOVEM::new_(MEM::addr_of(operand0),MEM::addr_of(operand1),result->type->size);
		result->is_lvalue=false;
	}else if(operand0->type->is_pointer() && operand1->type->is_pointer())
	{
		struct POINTER_TYPE* pointer0=static_cast<struct POINTER_TYPE*>(operand0->type);
		struct POINTER_TYPE* pointer1=static_cast<struct POINTER_TYPE*>(operand1->type);
		if(!is_qualifier_included(pointer0->type_,pointer1->type_))
		{
			::SCC_WARNNING(pos,"= :different qualifiers.");
		}
		if(pointer0->type_->is_compatible(pointer1->type_))
		{
			result=IC_MOVE::new_(operand0,operand1);
			result->type=operand0->type->unqualify();
			result->is_lvalue=false;
		}else if((pointer0->type_->is_void() && (pointer1->type_->is_object_type() || pointer1->type_->is_incomplete())) ||
			(pointer1->type_->is_void() && (pointer0->type_->is_object_type() || pointer0->type_->is_incomplete())))
		{
			result=IC_MOVE::new_(operand0,operand1);
			result->type=operand0->type->unqualify();
			result->is_lvalue=false;
		}else
		{
			::SCC_ERROR(pos,"= : uncompatible pointer type.");
			result=NULL;
		}
	}else if(operand0->type->is_pointer() && operand1->is_CONST() && operand1->type->is_integral_type() && static_cast<struct CONST*>(operand1)->is_zero())
	{
		operand1=IC_CVT::cast_to(operand1,operand0->type->unqualify());
		result=IC_MOVE::new_(operand0,operand1);
		result->type=operand0->type->unqualify();
		result->is_lvalue=false;
	}else
	{
		::SCC_ERROR(pos,"= : unassignable type.");
		result=NULL;
	}
	return result;
}

template<typename TYPE>
void compound_assignment_checker(struct exp* expr)
{
	expr->check_operands();
	struct ADDRESS* operand0=expr->operand_list[0]->addr;
	struct ADDRESS* operand1=expr->operand_list[1]->addr;

	struct ADDRESS* t=TYPE::check_operator(expr->pos,operand0,operand1);
	if(t==NULL)
	{
		expr->as_default();
	}else
	{
		t=exp_ASSIGN::check_operator(expr->pos,operand0,t);
		if(t==NULL)
		{
			expr->as_default();
		}else
			expr->addr=t;
	}
}
void exp_ADD_ASSIGN::check()
{
	compound_assignment_checker<exp_ADD>(this);
}
void exp_SUB_ASSIGN::check()
{
	compound_assignment_checker<exp_SUB>(this);
}
void exp_MUL_ASSIGN::check()
{
	compound_assignment_checker<exp_MUL>(this);
}
void exp_DIV_ASSIGN::check()
{
	compound_assignment_checker<exp_DIV>(this);
}
void exp_MOD_ASSIGN::check()
{
	compound_assignment_checker<exp_MOD>(this);
}
void exp_LSHIFT_ASSIGN::check()
{
	compound_assignment_checker<exp_LSHIFT>(this);
}
void exp_RSHIFT_ASSIGN::check()
{
	compound_assignment_checker<exp_RSHIFT>(this);
}
void exp_BITAND_ASSIGN::check()
{
	compound_assignment_checker<exp_BITAND>(this);
}
void exp_BITXOR_ASSIGN::check()
{
	compound_assignment_checker<exp_BITXOR>(this);
}
void exp_BITOR_ASSIGN::check()
{
	compound_assignment_checker<exp_BITOR>(this);
}

void exp_PRE_INC::check()
{
	this->check_operands();
	struct ADDRESS* operand0=this->operand_list[0]->addr;

	struct CONST* const1=new CONST(1);

	struct ADDRESS* t=exp_ADD::check_operator(pos,operand0,const1);
	if(t==NULL)
	{
		this->as_default();
	}else
	{
		t=exp_ASSIGN::check_operator(pos,operand0,t);
		if(t==NULL)
		{
			this->as_default();
		}else
			this->addr=t;
	}
}
void exp_PRE_DEC::check()
{
	this->check_operands();
	struct ADDRESS* operand0=this->operand_list[0]->addr;

	struct CONST* const1=new CONST(1);

	struct ADDRESS* t=exp_SUB::check_operator(pos,operand0,const1);
	if(t==NULL)
	{
		this->as_default();
	}else
	{
		t=exp_ASSIGN::check_operator(pos,operand0,t);
		if(t==NULL)
		{
			this->as_default();
		}else
			this->addr=t;
	}
}
void exp_POST_INC::check()
{
	this->check_operands();
	struct ADDRESS* operand0=this->operand_list[0]->addr;
	struct CONST* const1=new CONST(1);
	if(operand0->type->is_integral_type())
	{
		struct TYPE* type=integral_promotion(operand0->type);
		operand0=IC_CVT::cast_to(operand0,type);
	}
	struct TEMP* result=new TEMP();
	result->type=operand0->type;
	IC_MOVE::new_(result,operand0);
	struct ADDRESS* t=exp_ADD::check_operator(pos,operand0,const1);
	if(t==NULL)
	{
		this->as_default();
	}else
	{
		t=exp_ASSIGN::check_operator(pos,operand0,t);
		if(t==NULL)
		{
			this->as_default();
		}else
			this->addr=result;
	}
}
void exp_POST_DEC::check()
{
	this->check_operands();
	struct ADDRESS* operand0=this->operand_list[0]->addr;
	struct CONST* const1=new CONST(1);
	if(operand0->type->is_integral_type())
	{
		struct TYPE* type=integral_promotion(operand0->type);
		operand0=IC_CVT::cast_to(operand0,type);
	}
	struct TEMP* result=new TEMP();
	result->type=operand0->type;
	IC_MOVE::new_(result,operand0);
	struct ADDRESS* t=exp_SUB::check_operator(pos,operand0,const1);
	if(t==NULL)
	{
		this->as_default();
	}else
	{
		t=exp_ASSIGN::check_operator(pos,operand0,t);
		if(t==NULL)
		{
			this->as_default();
		}else
			this->addr=result;
	}
}