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

void exp_POS::check()
{
	this->check_operands();
	struct ADDRESS* operand0=this->operand_list[0]->addr;

	if(operand0->type->is_arithmetic_type())
	{
		if(operand0->type->is_integral_type())
		{
			struct TYPE*type=integral_promotion(operand0->type);
			operand0=IC_CVT::cast_to(operand0,type);
		}
		this->addr=operand0;
	}else
	{
		::SCC_ERROR(this->pos,"+(unary) : need arithmetic type.",this->lex_op.c_str());
		this->as_default();
	}
}
void exp_COMP::check()
{
	this->check_operands();
	struct ADDRESS* operand0=this->operand_list[0]->addr;

	if(operand0->type->is_integral_type())
	{
		this->addr=IC_COMP::new_(operand0);
	}else
	{
		::SCC_ERROR(this->pos,"~ : need integral type.",this->lex_op.c_str());
		this->as_default();
	}
}
void exp_NEG::check()
{
	this->check_operands();
	struct ADDRESS* operand0=this->operand_list[0]->addr;

	if(operand0->type->is_arithmetic_type())
	{
		if(static_cast<BASIC_TYPE*>(operand0->type)->is_uint32())
		{
			::SCC_WARNNING(pos,"unary minus operator applied to unsigned type, result still unsigned.");
		}
		this->addr=IC_NEG::new_(operand0);
	}else
	{
		::SCC_ERROR(this->pos,"%s(unary minus) : need arithmetic type.",this->lex_op.c_str());
		this->as_default();
	}
}
void exp_ADD::check()
{
	this->check_operands();
	struct ADDRESS* operand0=this->operand_list[0]->addr;
	struct ADDRESS* operand1=this->operand_list[1]->addr;
	struct ADDRESS* result;
	result=exp_ADD::check_operator(pos,operand0,operand1);
	if(result==NULL)
	{
		this->as_default();
	}else
		this->addr=result;
}
struct ADDRESS* exp_ADD::check_operator(std::string pos,struct ADDRESS* operand0,struct ADDRESS* operand1)
{
	struct ADDRESS* result;
	if(operand0->type->is_arithmetic_type() && operand1->type->is_arithmetic_type())
	{
		bool overflow=false;
		result=IC_ADD::new_(operand0,operand1,&overflow);
		if(overflow)
		{
			::SCC_WARNNING(pos,"+ : integral constant overflow.");
		}
	}else
	{
		if(operand1->type->is_pointer_to_object_type() && operand0->type->is_integral_type())
		{
			std::swap(operand0,operand1);
		}
		if( operand0->type->is_pointer_to_object_type() && operand1->type->is_integral_type() )
		{
			struct POINTER_TYPE* ptr=static_cast<struct POINTER_TYPE*>(operand0->type);
			bool overflow=false;
			struct CONST *scale=new CONST(ptr->type_->size);
			struct ADDRESS* offset=IC_MUL::new_(operand1,scale,&overflow);
			if(overflow)
			{
				::SCC_WARNNING(pos,"* : integral constant overflow.");
			}
			overflow=false;
			result=IC_ADD::new_(operand0,offset,&overflow);
			if(overflow)
			{
				::SCC_WARNNING(pos,"+ : integral constant overflow.");
			}
			result->type=ptr;
			result->is_lvalue=false;
		}else
		{
			::SCC_ERROR(pos,"+ : need both arithmetic type, or pointer and integral type");
			result=NULL;
		}
	}
	return result;
}
void exp_SUB::check()
{
	this->check_operands();
	struct ADDRESS* operand0=this->operand_list[0]->addr;
	struct ADDRESS* operand1=this->operand_list[1]->addr;
	struct ADDRESS* result;
	result=exp_SUB::check_operator(pos,operand0,operand1);
	if(result==NULL)
	{
		this->as_default();
	}else
		this->addr=result;
}
struct ADDRESS* exp_SUB::check_operator(std::string pos,struct ADDRESS* operand0,struct ADDRESS* operand1)
{
	struct ADDRESS* result;
	if(operand0->type->is_arithmetic_type() && operand1->type->is_arithmetic_type())
	{
		bool overflow=false;
		result=IC_SUB::new_(operand0,operand1,&overflow);
		if(overflow)
		{
			::SCC_WARNNING(pos,"- : integral constant overflow.");
		}
	}else
	{
		if( operand0->type->is_pointer_to_object_type() && operand1->type->is_integral_type() )
		{
			struct POINTER_TYPE* ptr=static_cast<struct POINTER_TYPE*>(operand0->type);
			bool overflow=false;
			struct CONST *scale=new CONST(ptr->type_->size);
			struct ADDRESS* offset=IC_MUL::new_(operand1,scale,&overflow);
			if(overflow)
			{
				::SCC_WARNNING(pos,"* : integral constant overflow.");
			}
			overflow=false;
			result=IC_SUB::new_(operand0,offset,&overflow);
			if(overflow)
			{
				::SCC_WARNNING(pos,"- : integral constant overflow.");
			}
			result->type=ptr;
			result->is_lvalue=false;
		}else if(operand0->type->is_pointer() && operand1->type->is_pointer())
		{
			struct POINTER_TYPE* ptr0=static_cast<struct POINTER_TYPE*>(operand0->type);
			struct POINTER_TYPE* ptr1=static_cast<struct POINTER_TYPE*>(operand1->type);
			if(ptr0->type_->is_compatible(ptr1->type_))
			{
				struct CONST *scale=new CONST(ptr0->type_->size);
				struct ADDRESS * pointer_difference=IC_SUB::new_(operand0,operand1,NULL);
				pointer_difference->type=TYPE::basic_type("int32");//cast to int32
				result=IC_DIV::new_(pointer_difference,scale);
				result->type=TYPE::basic_type("int32");
			}else
			{
				::SCC_ERROR(pos,"- : pointers to uncompatible type.");
				result=NULL;
			}
		}else
		{
			::SCC_ERROR(pos,"- : need both arithmetic type, or pointer and integral type,\
						or pointers to compatible type.");
			result=NULL;
		}
	}
	return result;
}
void exp_MUL::check()
{
	this->check_operands();
	struct ADDRESS* operand0=this->operand_list[0]->addr;
	struct ADDRESS* operand1=this->operand_list[1]->addr;
	struct ADDRESS* result;
	result=exp_MUL::check_operator(pos,operand0,operand1);
	if(result==NULL)
	{
		this->as_default();
	}else
		this->addr=result;
}
struct ADDRESS* exp_MUL::check_operator(std::string pos,struct ADDRESS* operand0,struct ADDRESS* operand1)
{
	struct ADDRESS* result;
	if(operand0->type->is_arithmetic_type() && operand1->type->is_arithmetic_type())
	{
		bool overflow=false;
		result=IC_MUL::new_(operand0,operand1,&overflow);
		if(overflow)
		{
			::SCC_WARNNING(pos,"* : integral constant overflow.");
		}
		return result;
	}else
	{
		::SCC_ERROR(pos,"* : need arithmetic type.");
		return NULL;
	}
}
void exp_DIV::check()
{
	this->check_operands();
	struct ADDRESS* operand0=this->operand_list[0]->addr;
	struct ADDRESS* operand1=this->operand_list[1]->addr;
	struct ADDRESS* result;
	result=exp_DIV::check_operator(pos,operand0,operand1);
	if(result==NULL)
	{
		this->as_default();
	}else
		this->addr=result;
}
struct ADDRESS* exp_DIV::check_operator(std::string pos,struct ADDRESS* operand0,struct ADDRESS* operand1)
{
	struct ADDRESS* result;
	if(operand0->type->is_arithmetic_type() && operand1->type->is_arithmetic_type())
	{
		if(operand1->is_CONST() && static_cast<struct CONST*>(operand1)->is_zero())
		{
			::SCC_WARNNING(pos,"/ : divide by 0.");
		}
		result=IC_DIV::new_(operand0,operand1);
		return result;
	}else
	{
		::SCC_ERROR(pos,"/ : need arithmetic type.");
		return NULL;
	}
}
void exp_MOD::check()
{
	this->check_operands();
	struct ADDRESS* operand0=this->operand_list[0]->addr;
	struct ADDRESS* operand1=this->operand_list[1]->addr;
	struct ADDRESS* result;
	result=exp_MOD::check_operator(pos,operand0,operand1);
	if(result==NULL)
	{
		this->as_default();
	}else
		this->addr=result;
}
struct ADDRESS* exp_MOD::check_operator(std::string pos,struct ADDRESS* operand0,struct ADDRESS* operand1)
{
	struct ADDRESS* result;
	if(operand0->type->is_integral_type() && operand1->type->is_integral_type())
	{
		if(operand1->is_CONST() && static_cast<struct CONST*>(operand1)->is_zero())
		{
			::SCC_WARNNING(pos,"%s : mod by 0.","%");
		}
		result=IC_MOD::new_(operand0,operand1);
		return result;
	}else
	{
		::SCC_ERROR(pos,"%s : need integral type.","%");
		return NULL;
	}
}

void exp_LSHIFT::check()
{
	this->check_operands();
	struct ADDRESS* operand0=this->operand_list[0]->addr;
	struct ADDRESS* operand1=this->operand_list[1]->addr;
	struct ADDRESS* result;
	result=exp_LSHIFT::check_operator(pos,operand0,operand1);
	if(result==NULL)
	{
		this->as_default();
	}else
		this->addr=result;
}
struct ADDRESS* exp_LSHIFT::check_operator(std::string pos,struct ADDRESS* operand0,struct ADDRESS* operand1)
{
	struct ADDRESS* result;
	if(operand0->type->is_integral_type() && operand1->type->is_integral_type())
	{
		if(operand1->is_CONST() && static_cast<struct CONST*>(operand1)->as_uint32()>31)
		{
			::SCC_WARNNING(pos,"<< : shift count negative or too big, undefined behavior.");
		}
		result=IC_LSHIFT::new_(operand0,operand1);
		return result;
	}else
	{
		::SCC_ERROR(pos,"<< : need integral type.");
		return NULL;
	}
}
void exp_RSHIFT::check()
{
	this->check_operands();
	struct ADDRESS* operand0=this->operand_list[0]->addr;
	struct ADDRESS* operand1=this->operand_list[1]->addr;
	struct ADDRESS* result;
	result=exp_RSHIFT::check_operator(pos,operand0,operand1);
	if(result==NULL)
	{
		this->as_default();
	}else
		this->addr=result;
}
struct ADDRESS* exp_RSHIFT::check_operator(std::string pos,struct ADDRESS* operand0,struct ADDRESS* operand1)
{
	struct ADDRESS* result;
	if(operand0->type->is_integral_type() && operand1->type->is_integral_type())
	{
		if(operand1->is_CONST() && static_cast<struct CONST*>(operand1)->as_uint32()>31)
		{
			::SCC_WARNNING(pos,">> : shift count negative or too big, undefined behavior.");
		}
		result=IC_RSHIFT::new_(operand0,operand1);
		return result;
	}else
	{
		::SCC_ERROR(pos,">> : need integral type.");
		return NULL;
	}
}

void exp_BITAND::check()
{
	this->check_operands();
	struct ADDRESS* operand0=this->operand_list[0]->addr;
	struct ADDRESS* operand1=this->operand_list[1]->addr;
	struct ADDRESS* result;
	result=exp_BITAND::check_operator(pos,operand0,operand1);
	if(result==NULL)
	{
		this->as_default();
	}else
		this->addr=result;
}
struct ADDRESS* exp_BITAND::check_operator(std::string pos,struct ADDRESS* operand0,struct ADDRESS* operand1)
{
	struct ADDRESS* result;
	if(operand0->type->is_integral_type() && operand1->type->is_integral_type())
	{
		result=IC_BITAND::new_(operand0,operand1);
		return result;
	}else
	{
		::SCC_ERROR(pos,"& : need integral type.");
		return NULL;
	}
}
void exp_BITXOR::check()
{
	this->check_operands();
	struct ADDRESS* operand0=this->operand_list[0]->addr;
	struct ADDRESS* operand1=this->operand_list[1]->addr;
	struct ADDRESS* result;
	result=exp_BITXOR::check_operator(pos,operand0,operand1);
	if(result==NULL)
	{
		this->as_default();
	}else
		this->addr=result;
}
struct ADDRESS* exp_BITXOR::check_operator(std::string pos,struct ADDRESS* operand0,struct ADDRESS* operand1)
{
	struct ADDRESS* result;
	if(operand0->type->is_integral_type() && operand1->type->is_integral_type())
	{
		result=IC_BITXOR::new_(operand0,operand1);
		return result;
	}else
	{
		::SCC_ERROR(pos,"^ : need integral type.");
		return NULL;
	}
}
void exp_BITOR::check()
{
	this->check_operands();
	struct ADDRESS* operand0=this->operand_list[0]->addr;
	struct ADDRESS* operand1=this->operand_list[1]->addr;
	struct ADDRESS* result;
	result=exp_BITOR::check_operator(pos,operand0,operand1);
	if(result==NULL)
	{
		this->as_default();
	}else
		this->addr=result;
}
struct ADDRESS* exp_BITOR::check_operator(std::string pos,struct ADDRESS* operand0,struct ADDRESS* operand1)
{
	struct ADDRESS* result;
	if(operand0->type->is_integral_type() && operand1->type->is_integral_type())
	{
		result=IC_BITOR::new_(operand0,operand1);
		return result;
	}else
	{
		::SCC_ERROR(pos,"| : need integral type.");
		return NULL;
	}
}
