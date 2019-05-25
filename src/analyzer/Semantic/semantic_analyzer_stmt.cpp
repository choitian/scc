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

#include "..\lib\common.h"
#include "semantic_analyzer.h"
#include "..\Syntax_Tree\syntax_tree.h"
#include "..\Syntax_Tree\syntax_tree_decl.h"
#include "..\Syntax_Tree\syntax_tree_stmt.h"
#include "..\Syntax_Tree\syntax_tree_exp.h"
#include "Symbol_Table\type.h"
#include "Symbol_Table\identifier.h"
#include "Symbol_Table\scope.h"
#include "expression\IC.h"

namespace
{
	bool is_integral_const(struct exp *expression,unsigned int *value)
	{
		static unsigned int i=0;
		if(value!=NULL)
			*value=i++;
		return true;
	}
	bool is_integral_type(struct exp *expression)
	{
		return true;
	}
	bool is_scalar_type(struct exp *expression)
	{
		return true;
	}
}
struct BASIC_TYPE * integral_promotion(struct TYPE* fst);
void stmt_COMPOUND::check()
{
	SCOPE::current->enter_scope();
	struct syntax_node* node;
	struct DECLARATION*  casted;
	for(size_t i=0;i<this->decl_list.size();i++)
	{
		node=this->decl_list.at(i);
		casted=static_cast<struct DECLARATION*>(node);
		casted->check();
	}
	for(size_t i=0;i<this->stmt_list.size();i++)
	{
		node=this->stmt_list.at(i);
		node->check();
	}
	SCOPE::current->exit_scope();
}
void stmt_COMPOUND::funtion_body_check(struct SCOPE* prototype_scope)
{
	prototype_scope->as_current();
	struct syntax_node* node;
	struct DECLARATION*  casted;

	IC_INS_BLOCK::new_()->start();
	iFUNCTION::current_definition->end_block=IC_INS_BLOCK::new_();
	for(size_t i=0;i<this->decl_list.size();i++)
	{
		node=this->decl_list.at(i);
		casted=static_cast<struct DECLARATION*>(node);
		casted->check();
	}
	for(size_t i=0;i<this->stmt_list.size();i++)
	{
		node=this->stmt_list.at(i);
		node->check();
	}
	iFUNCTION::current_definition->end_block->start();
	NOP::new_();//add a nop to end block as mark.
	SCOPE::current->exit_scope();
}
void stmt_LABEL::check()
{
	struct iLABEL *label=iFUNCTION::current_definition->function_scope.get_label(this->name);
	if(label->defined)
	{
		::SCC_ERROR(pos,"label : %s : redefinition.",name.c_str());
	}else
	{
		label->defined=true;
		label->iblock->start();
	}
	this->statement->check();
}
void stmt_GOTO::check()
{
	JUMP::new_(iFUNCTION::current_definition->function_scope.get_label(this->name)->iblock);
}

void stmt_IF::check()
{

	if(this->false_statement==NULL)
	{
		struct IC_INS_BLOCK *next=IC_INS_BLOCK::new_();
		this->expression->check_as_branch(NULL,next);
		this->true_statement->check();
		next->start();
	}else
	{
		struct IC_INS_BLOCK *iffalse=IC_INS_BLOCK::new_();
		struct IC_INS_BLOCK *next=IC_INS_BLOCK::new_();
		this->expression->check_as_branch(NULL,iffalse);
		this->true_statement->check();
		JUMP::new_(next);
		iffalse->start();
		this->false_statement->check();
		next->start();
	}
}
bool stmt_SWITCH::insert_case(std::string value,struct stmt_CASE* case_)
{
	if(this->case_map.find(value)!=this->case_map.end())
	{
		return false;
	}else
	{
		case_list.push_back(case_);
		this->case_map.insert(std::pair<std::string,struct stmt_CASE*>(value,case_));
		return true;
	}
}
void stmt_SWITCH::check()
{
	struct IC_INS_BLOCK *the_end=IC_INS_BLOCK::new_();
	struct IC_INS_BLOCK *switch_table=IC_INS_BLOCK::new_();
	iFUNCTION::current_definition->function_scope.break_points.push(the_end);
	iFUNCTION::current_definition->function_scope.switch_stack.push_back(this);
	this->expression->check();
	this->expression->adjust(this->op,false);
	if(!this->expression->addr->type->is_integral_type())
	{
		::SCC_ERROR(pos,"switch expression not integral.");
	}else
	{
		this->expression->addr=IC_CVT::cast_to(this->expression->addr,integral_promotion(this->expression->addr->type));
		switch_table->start();
		IC_INS_BLOCK::new_()->start();
		this->statement->check();
		the_end->start();

		iFUNCTION::current_definition->set_current_block(switch_table);
		for(size_t i=0;i< this->case_list.size();i++)
		{
			struct stmt_CASE* case_=this->case_list.at(i);
			IJUMP::new_("==",this->expression->addr,case_->expression->addr,case_->label,NULL);
		}
		if(this->stmt_default!=NULL)
			JUMP::new_(this->stmt_default->label);
		else
			JUMP::new_(the_end);
		iFUNCTION::current_definition->reset_current_block();
	}
	iFUNCTION::current_definition->function_scope.switch_stack.pop_back();
	iFUNCTION::current_definition->function_scope.break_points.pop();
}
void stmt_CASE::check()
{

	if(iFUNCTION::current_definition->function_scope.switch_stack.empty())
	{
		::SCC_ERROR(pos,"illegal case.");
	}else
	{
		struct stmt_SWITCH* outter_switch=iFUNCTION::current_definition->function_scope.switch_stack.back();

		this->expression->check();
		this->expression->adjust(this->op,false);
		if(!this->expression->addr->is_CONST() || !this->expression->addr->type->is_integral_type())
			::SCC_ERROR(pos,"case expression not integral constant.");
		else
		{
			this->expression->addr=IC_CVT::cast_to(this->expression->addr,outter_switch->expression->addr->type);
			if(!outter_switch->insert_case(this->expression->addr->toString(),this))
			{
				::SCC_ERROR(pos,"case value '%s' already used.",this->expression->addr->toString().c_str());
			}
		}
	}
	this->label=IC_INS_BLOCK::new_();
	this->label->start();
	this->statement->check();
}
void stmt_DEFAULT::check()
{
	if(iFUNCTION::current_definition->function_scope.switch_stack.empty())
	{
		::SCC_ERROR(pos,"illegal default.");
	}else
	{
		struct stmt_SWITCH* outter_switch=iFUNCTION::current_definition->function_scope.switch_stack.back();
		if(outter_switch->stmt_default!=NULL)
			::SCC_ERROR(pos,"more than one default.");
		else
			outter_switch->stmt_default=this;
	}
	this->label=IC_INS_BLOCK::new_();
	this->label->start();
	this->statement->check();
}

void stmt_WHILE::check()
{
/*
goto test
stmt:
stmt.check()
test:
expr.check(stmt,NULL)
end:
*/
	struct IC_INS_BLOCK *the_test=IC_INS_BLOCK::new_();
	struct IC_INS_BLOCK *the_stmt=IC_INS_BLOCK::new_();
	struct IC_INS_BLOCK *the_end=IC_INS_BLOCK::new_();
	iFUNCTION::current_definition->function_scope.continue_points.push(the_test);
	iFUNCTION::current_definition->function_scope.break_points.push(the_end);
	{
		JUMP::new_(the_test);
		the_stmt->start();
		this->statement->check();
		the_test->start();
		this->expression->check_as_branch(the_stmt,NULL);
		the_end->start();
	}
	iFUNCTION::current_definition->function_scope.break_points.pop();
	iFUNCTION::current_definition->function_scope.continue_points.pop();
}
void stmt_DO_WHILE::check()
{
/*
stmt:
stmt.check()
test:
expr.check(stmt,NULL)
end:
*/
	struct IC_INS_BLOCK *the_test=IC_INS_BLOCK::new_();
	struct IC_INS_BLOCK *the_stmt=IC_INS_BLOCK::new_();
	struct IC_INS_BLOCK *the_end=IC_INS_BLOCK::new_();
	iFUNCTION::current_definition->function_scope.continue_points.push(the_test);
	iFUNCTION::current_definition->function_scope.break_points.push(the_end);
	{
		the_stmt->start();
		this->statement->check();
		the_test->start();
		this->expression->check_as_branch(the_stmt,NULL);
		the_end->start();
	}
	iFUNCTION::current_definition->function_scope.break_points.pop();
	iFUNCTION::current_definition->function_scope.continue_points.pop();
}
void stmt_FOR::check()
{
/*
initialization_expression.check();
goto test
stmt:
stmt.check()
continue:
reinitialization_expression.check();
test:
controlling_expression.check(stmt,NULL)
end:
*/
	struct IC_INS_BLOCK *the_test=IC_INS_BLOCK::new_();
	struct IC_INS_BLOCK *the_stmt=IC_INS_BLOCK::new_();
	struct IC_INS_BLOCK *the_contine=IC_INS_BLOCK::new_();
	struct IC_INS_BLOCK *the_end=IC_INS_BLOCK::new_();
	iFUNCTION::current_definition->function_scope.continue_points.push(the_contine);
	iFUNCTION::current_definition->function_scope.break_points.push(the_end);
	{
		this->initialization_expression->check();
		JUMP::new_(the_test);
		the_stmt->start();
		this->statement->check();
		the_contine->start();
		this->reinitialization_expression->check();
		the_test->start();
		this->controlling_expression->check_as_branch(the_stmt,NULL);
		the_end->start();
	}
	iFUNCTION::current_definition->function_scope.break_points.pop();
	iFUNCTION::current_definition->function_scope.continue_points.pop();
}

void stmt_CONTINUE::check()
{
	if(iFUNCTION::current_definition->function_scope.continue_points.empty())
	{
		::SCC_ERROR(pos,"illegal contine.");
	}else
		JUMP::new_(iFUNCTION::current_definition->function_scope.continue_points.top());
}
void stmt_BREAK::check()
{
	if(iFUNCTION::current_definition->function_scope.break_points.empty())
	{
		::SCC_ERROR(pos,"illegal break.");
	}else
		JUMP::new_(iFUNCTION::current_definition->function_scope.break_points.top());
}
void stmt_EXPRESSION::check()
{
	if(this->expression!=NULL)
		this->expression->check();
}
void stmt_RETURN::check()
{
	struct TYPE* return_type=iFUNCTION::current_definition->return_type();
	struct IC_INS_BLOCK* end_block=iFUNCTION::current_definition->end_block;
	if(this->expression!=NULL)
	{
		this->expression->check();
		struct ADDRESS* operand0=this->expression->addr;
		if(return_type->is_arithmetic_type() && operand0->type->is_arithmetic_type())
		{
			operand0=IC_CVT::cast_to(operand0,return_type);
			RET::new_(operand0);
			JUMP::new_(end_block);
		}else if(is_compatible_struct_or_union(return_type,operand0->type))
		{
			RET::new_(operand0);
			JUMP::new_(end_block);
		}else if(return_type->is_pointer() && operand0->type->is_pointer())
		{
			struct POINTER_TYPE* return_type_p=static_cast<struct POINTER_TYPE*>(return_type);
			struct POINTER_TYPE* operand0_p=static_cast<struct POINTER_TYPE*>(operand0->type);
			if(operand0_p->type_->is_compatible(return_type_p->type_))
			{
				operand0=IC_CVT::cast_to(operand0,return_type);
				RET::new_(operand0);
				JUMP::new_(end_block);
			}
			if((return_type_p->type_->is_void() && (operand0_p->type_->is_object_type() || operand0_p->type_->is_incomplete())) ||
				(operand0_p->type_->is_void() && (return_type_p->type_->is_object_type() || return_type_p->type_->is_incomplete())))
			{
				operand0=IC_CVT::cast_to(operand0,return_type);
				RET::new_(operand0);
				JUMP::new_(end_block);
			}
		}else if(return_type->is_pointer() && operand0->is_CONST() && operand0->type->is_scalar_type() && static_cast<struct CONST*>(operand0)->is_zero())
		{
			operand0=IC_CVT::cast_to(operand0,return_type);
			RET::new_(operand0);
			JUMP::new_(end_block);
		}else
		{
			::SCC_ERROR(pos,"return : return type not match the function type.");
		}
	}else
	{
		if(!return_type->is_void())
			::SCC_ERROR(pos,"%s : function must return a value.",iFUNCTION::current_definition->name.c_str());
	}
}