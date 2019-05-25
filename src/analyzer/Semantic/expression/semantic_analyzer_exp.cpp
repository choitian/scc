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

void exp_COMMA::check()
{
	this->check_operands();
	struct ADDRESS* operand0=this->operand_list[0]->addr;
	struct ADDRESS* operand1=this->operand_list[1]->addr;

	this->addr=operand1;
}

static struct TYPE* union_pointer_qualifiers(struct TYPE* type0,struct TYPE* type1)
{
	return TYPE::pointer_type(type0->unqualify()->qualify(type0->is_const() || type1->is_const(),
		type0->is_volatile() || type1->is_volatile()));
}
struct ADDRESS* exp_CONDI::options_check(struct IC_INS_BLOCK *true_adjust,struct IC_INS_BLOCK *false_adjust,struct ADDRESS* true_addr,struct ADDRESS* false_addr)
{
	struct ADDRESS* result;
	if(true_addr->type->is_arithmetic_type() && false_addr->type->is_arithmetic_type())
	{
		struct TYPE* type=usual_arithmetic_conversion(true_addr->type,false_addr->type);
		result=MEM::local_temp(type);
		iFUNCTION::current_definition->set_current_block(true_adjust);
		true_addr=IC_CVT::cast_to(true_addr,result->type);
		IC_MOVE::new_(result,true_addr);
		iFUNCTION::current_definition->set_current_block(false_adjust);
		false_addr=IC_CVT::cast_to(false_addr,result->type);
		IC_MOVE::new_(result,false_addr);
		
	}else if(is_compatible_struct_or_union(true_addr->type,false_addr->type))
	{
		result=MEM::local_temp(true_addr->type);
		iFUNCTION::current_definition->set_current_block(true_adjust);
		IC_MOVEM::new_(MEM::addr_of(result),MEM::addr_of(true_addr),result->type->size);
		iFUNCTION::current_definition->set_current_block(false_adjust);
		IC_MOVEM::new_(MEM::addr_of(result),MEM::addr_of(false_addr),result->type->size);
	}else if(true_addr->type->is_pointer() && false_addr->type->is_pointer())
	{
		struct POINTER_TYPE* true_pointer=static_cast<struct POINTER_TYPE*>(true_addr->type);
		struct POINTER_TYPE* false_pointer=static_cast<struct POINTER_TYPE*>(false_addr->type);
		bool is_const=true_pointer->type_->is_const() || false_pointer->type_->is_const();
		bool is_volatile=true_pointer->type_->is_volatile() || false_pointer->type_->is_volatile();
		if(true_pointer->type_->is_compatible(false_pointer->type_))
		{
			result=MEM::local_temp(TYPE::pointer_type(true_pointer->type_->unqualify()->qualify(is_const,is_volatile)));
			iFUNCTION::current_definition->set_current_block(true_adjust);
			IC_MOVE::new_(result,true_addr);
			iFUNCTION::current_definition->set_current_block(false_adjust);
		}else if((true_pointer->type_->is_void() && (false_pointer->type_->is_object_type() || false_pointer->type_->is_incomplete())) ||
			(false_pointer->type_->is_void() && (true_pointer->type_->is_object_type() || true_pointer->type_->is_incomplete())))
		{
			result=MEM::local_temp(TYPE::pointer_type(TYPE::void_type()->qualify(is_const,is_volatile)));
			iFUNCTION::current_definition->set_current_block(true_adjust);
			if(!true_pointer->type_->is_void())
			{
				true_pointer->type_=TYPE::void_type();
			}
			IC_MOVE::new_(result,true_addr);
			iFUNCTION::current_definition->set_current_block(false_adjust);
			if(!false_pointer->type_->is_void())
			{
				true_pointer->type_=TYPE::void_type();
			}
			IC_MOVE::new_(result,false_addr);
		}else
		{
			::SCC_ERROR(pos,"'?:' : operands are incompatible.");
			result=NULL;
		}
	}else if( (true_addr->type->is_pointer() && false_addr->is_CONST() && false_addr->type->is_integral_type() && static_cast<struct CONST*>(false_addr)->is_zero()) ||
			(false_addr->type->is_pointer() && true_addr->is_CONST() && true_addr->type->is_integral_type() && static_cast<struct CONST*>(true_addr)->is_zero()) )
	{
		if(true_addr->is_CONST())
		{
			result=MEM::local_temp(false_addr->type);
		}else
			result=MEM::local_temp(true_addr->type);
		iFUNCTION::current_definition->set_current_block(true_adjust);
		true_addr=IC_CVT::cast_to(true_addr,result->type);
		IC_MOVE::new_(result,true_addr);
		iFUNCTION::current_definition->set_current_block(false_adjust);
		false_addr=IC_CVT::cast_to(false_addr,result->type);
		IC_MOVE::new_(result,false_addr);
	}else
	{
		::SCC_ERROR(pos,"'?:' : operands are incompatible.");
		result=NULL;
	}
	iFUNCTION::current_definition->reset_current_block();
	return result;
}
bool exp_CONDI::can_constant_fold()
{
	struct exp* test_exp=this->operand_list[0];
	struct exp* true_exp=this->operand_list[1];
	struct exp* false_exp=this->operand_list[2];
	bool discard_ins_old=iFUNCTION::current_definition->discard_ins;
	iFUNCTION::current_definition->discard_ins=true;
	test_exp->check();
	test_exp->adjust(this->op,false);
	true_exp->check();
	true_exp->adjust(this->op,false);
	false_exp->check();
	false_exp->adjust(this->op,false);
	iFUNCTION::current_definition->discard_ins=discard_ins_old;
	if(test_exp->addr->is_CONST() && true_exp->addr->is_CONST() && true_exp->addr->type->is_arithmetic_type()&&
		false_exp->addr->is_CONST() && false_exp->addr->type->is_arithmetic_type())
	{
		struct TYPE* type=usual_arithmetic_conversion(true_exp->addr->type,false_exp->addr->type);
		if(!static_cast<struct CONST*>(test_exp->addr)->is_zero())
		{
			this->addr=IC_CVT::cast_to(true_exp->addr,type);
		}else
		{
			this->addr=IC_CVT::cast_to(false_exp->addr,type);
		}
		return true;
	}
	return false;
}
void exp_CONDI::check()
{
/*
test.check(NULL,false);
r=true_exp.check();
jump next
false:
r=false_exp.check();
next:
*/
	if(can_constant_fold())
		return;
	struct exp* test_exp=this->operand_list[0];
	struct exp* true_exp=this->operand_list[1];
	struct exp* false_exp=this->operand_list[2];

	struct IC_INS_BLOCK *iffalse=IC_INS_BLOCK::new_();
	struct IC_INS_BLOCK *next=IC_INS_BLOCK::new_();
	struct IC_INS_BLOCK *true_adjust=IC_INS_BLOCK::new_();
	struct IC_INS_BLOCK *false_adjust=IC_INS_BLOCK::new_();

	test_exp->check_as_branch(NULL,iffalse);
	true_exp->check();
	true_exp->adjust(this->op,false);
	true_adjust->start();
	IC_INS_BLOCK::new_()->start();
	JUMP::new_(next);
	iffalse->start();
	false_exp->check();
	false_exp->adjust(this->op,false);
	false_adjust->start();
	IC_INS_BLOCK::new_()->start();
	next->start();
//option constraints
	struct ADDRESS* result=options_check(true_adjust,false_adjust,true_exp->addr,false_exp->addr);
	if(result==NULL)
	{
		this->as_default();
	}else
		this->addr=result;
}
struct ADDRESS* exp_CALL::check_argument(size_t argument_index,struct TYPE* parameter_type,struct ADDRESS* argument)
{
	if(parameter_type->is_arithmetic_type() && argument->type->is_arithmetic_type())
	{
		argument=IC_CVT::cast_to(argument,parameter_type);
	}else if(is_compatible_struct_or_union(parameter_type,argument->type))
	{
	}else if(parameter_type->is_pointer() && argument->type->is_pointer())
	{
		struct POINTER_TYPE* parameter_type_p=static_cast<struct POINTER_TYPE*>(parameter_type);
		struct POINTER_TYPE* argment_p=static_cast<struct POINTER_TYPE*>(argument->type);
		if(argment_p->type_->is_compatible(parameter_type_p->type_))
		{
			argument=IC_CVT::cast_to(argument,parameter_type);
		}
		if((parameter_type_p->type_->is_void() && (argment_p->type_->is_object_type() || argment_p->type_->is_incomplete())) ||
			(argment_p->type_->is_void() && (parameter_type_p->type_->is_object_type() || parameter_type_p->type_->is_incomplete())))
		{
			argument=IC_CVT::cast_to(argument,parameter_type);
		}
	}else if(parameter_type->is_pointer() && argument->is_CONST() && argument->type->is_scalar_type() && static_cast<struct CONST*>(argument)->is_zero())
	{
		argument=IC_CVT::cast_to(argument,parameter_type);
	}else
	{
		::SCC_ERROR(pos,"argument %d is incompatible with paramenter type.",argument_index+1);
	}
	return argument;
}
void exp_CALL::check()
{
	this->designator->check();
	this->designator->adjust(this->op,false);
	struct ADDRESS* fun_designator=this->designator->addr;
	struct ADDRESS* argument;
	if(fun_designator->type->is_pointer() && static_cast<struct POINTER_TYPE*>(fun_designator->type)->type_->is_function())
	{
		struct TYPE_PROTOTYPE* prototype;
		struct TYPE_FUNCTION* function_type;
		function_type=static_cast<struct TYPE_FUNCTION*>(static_cast<struct POINTER_TYPE*>(fun_designator->type)->type_);
		prototype=function_type->prototype_;
		struct TYPE* corresponding_paramenter;
		size_t argument_index;
		size_t stack_size=0;
		for(size_t i=0;i<this->operand_list.size();i++)//corresponding
		{

			argument_index=this->operand_list.size()-i-1;//last ith argument.
			this->operand_list.at(argument_index)->check();
			this->operand_list.at(argument_index)->adjust(this->op,false);
			argument=this->operand_list.at(argument_index)->addr;
			if(argument_index >= prototype->list_.size())
			{
				if(!prototype->is_variadic_)
				{
					::SCC_ERROR(pos,"too many argument to call a function.");
				}else
				{
					if(argument->type->is_arithmetic_type())
					{
						if(argument->type->is_integral_type())
							argument=IC_CVT::cast_to(argument,integral_promotion(argument->type));
						else
							argument=IC_CVT::cast_to(argument,TYPE::basic_type("float64"));
					}
				}
			}else
			{
				corresponding_paramenter=prototype->list_.at(argument_index)->type;
				argument=check_argument(argument_index,corresponding_paramenter,argument);
			}
			PARAM::new_(argument);
			stack_size+=argument->type->size;
			stack_size=ALIGN(stack_size,4);
		}
		if(prototype->list_.size() > operand_list.size())
		{
			::SCC_ERROR(pos,"too few argument to call a function.");
		}
		if(function_type->type_->is_struct_or_union())
		{
			struct MEM* mem=MEM::local_temp(function_type->type_);
			struct ADDRESS* addr=MEM::addr_of(mem);
			stack_size+=addr->type->size;
			PARAM::new_(addr);
			CALL::new_(function_type->type_,fun_designator,stack_size);
			this->addr=mem;
		}else
		{
			this->addr=CALL::new_(function_type->type_,fun_designator,stack_size);
		}
		this->addr->is_lvalue=false;
	}else
	{
		::SCC_ERROR(pos,"call: need a function type.");
		this->as_default();
	}
}
void exp_SIZE_OF_TYPE::check()
{
	struct TYPE* type=this->type_name->get_type();
	if(type->is_function())
	{
		::SCC_ERROR(pos,"sizeof: function type,illegal operand.");
		this->as_default();
	}else if(type->is_incomplete())
	{
		if(type->is_void())
			::SCC_ERROR(pos,"sizeof: 'void',illegal operand.");
		else
			::SCC_ERROR(pos,"sizeof: incomplete struct or union.");
		this->as_default();
	}else
	{
		struct CONST *c=new CONST();
		c->value.uint32=type->size;
		c->type=TYPE::basic_type("uint32");
		this->addr=c;
	}
}
void exp_SIZE_OF_OBJ::check()
{
	struct exp* obj=this->operand_list[0];
	bool discard_ins_old=iFUNCTION::current_definition->discard_ins;
	iFUNCTION::current_definition->discard_ins=true;
	obj->check();
	struct TYPE* type=obj->addr->type;
	iFUNCTION::current_definition->discard_ins=discard_ins_old;	
	if(type->is_function())
	{
		::SCC_ERROR(pos,"sizeof: function type,illegal operand.");
		this->as_default();
	}else if(type->is_incomplete())
	{
		if(type->is_void())
			::SCC_ERROR(pos,"sizeof: 'void',illegal operand.");
		else
			::SCC_ERROR(pos,"sizeof: incomplete struct or union.");
		this->as_default();
	}else if(obj->addr->is_BIT_FIELD())
	{
		::SCC_ERROR(pos,"sizeof: bit-field,illegal operand.");
		this->as_default();
	}else
	{
		struct CONST *c=new CONST();
		c->value.uint32=type->size;
		c->type=TYPE::basic_type("uint32");
		this->addr=c;
	}
}