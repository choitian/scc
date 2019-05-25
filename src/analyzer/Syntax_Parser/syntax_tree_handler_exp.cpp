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

#include "Syntax_Parser .h"
#include "syntax_tree_builder.h"
namespace
{
	void getTERMINAL_EXTRA(std::string *str,struct syntax_node* node)
	{
		std::string tok;
		std::string extra;
		if(node->is_terminal(&tok,&extra))
		{
			*str=extra;
		}else
			assert(0);
	}
}




template<typename TYPE>
syntax_node* exp_template(struct syntax_stack* stack)
{
	TYPE* node=new TYPE;

	for(size_t i=0;i<stack->current_production()->nonterminal_list.size();i++)
	{
		node->add_operand(stack->nonterminal(i));
	}

	stack->replace(node);
	return node;
}
struct syntax_node* _exp_CALL(struct syntax_stack* stack)
{
	struct exp_CALL* node=new struct exp_CALL();
	node->designator=static_cast<struct exp*>(stack->production_node(0));
	std::vector<struct syntax_node*> argument_list;
	if(stack->current_production()->len==4)
		LIST_node::off_list(stack->production_node(2),&argument_list);
	for(size_t i=0;i< argument_list.size();i++)
	{
		node->add_operand(argument_list.at(i));
	}

	stack->replace(node);
	return node;
}
template<typename TYPE>
syntax_node* _exp_MBSL(struct syntax_stack* stack)
{
	TYPE* node=new TYPE;
	node->add_operand(stack->production_node(0));
	getTERMINAL_EXTRA(&node->member,stack->top(0));

	stack->replace(node);
	return node;
}
template<typename TYPE>
syntax_node* _exp_TERMINAL(struct syntax_stack* stack)
{
	TYPE* node=new TYPE;
	getTERMINAL_EXTRA(&node->lexical_value,stack->production_node(0));
	stack->replace(node);
	return node;
}

syntax_node* _exp_CAST(struct syntax_stack* stack)
{
	struct exp_CAST* node=new struct exp_CAST;
	node->type_name=static_cast<struct TYPE_NAME_DECLARATION*>(stack->nonterminal(0));
	node->expr=static_cast<struct exp*>(stack->nonterminal(1));

	stack->replace(node);
	return node;
}
syntax_node* _exp_SIZE_OF_TYPE(struct syntax_stack* stack)
{
	struct exp_SIZE_OF_TYPE* node=new struct exp_SIZE_OF_TYPE;
	node->type_name=static_cast<struct TYPE_NAME_DECLARATION*>(stack->nonterminal(0));

	stack->replace(node);
	return node;
}

void syntax_tree_builder::register_handler_exp()
{
	registered_handler_map_["exp_COMMA"]=exp_template<exp_COMMA>;
	registered_handler_map_["exp_ASSIGN"]=exp_template<exp_ASSIGN>;
	registered_handler_map_["exp_MUL_ASSIGN"]=exp_template<exp_MUL_ASSIGN>;
	registered_handler_map_["exp_DIV_ASSIGN"]=exp_template<exp_DIV_ASSIGN>;
	registered_handler_map_["exp_MOD_ASSIGN"]=exp_template<exp_MOD_ASSIGN>;
	registered_handler_map_["exp_ADD_ASSIGN"]=exp_template<exp_ADD_ASSIGN>;
	registered_handler_map_["exp_SUB_ASSIGN"]=exp_template<exp_SUB_ASSIGN>;
	registered_handler_map_["exp_LSHIFT_ASSIGN"]=exp_template<exp_LSHIFT_ASSIGN>;
	registered_handler_map_["exp_RSHIFT_ASSIGN"]=exp_template<exp_RSHIFT_ASSIGN>;
	registered_handler_map_["exp_BITAND_ASSIGN"]=exp_template<exp_BITAND_ASSIGN>;
	registered_handler_map_["exp_BITXOR_ASSIGN"]=exp_template<exp_BITXOR_ASSIGN>;
	registered_handler_map_["exp_BITOR_ASSIGN"]=exp_template<exp_BITOR_ASSIGN>;

	registered_handler_map_["exp_CONDI"]=exp_template<exp_CONDI>;

	registered_handler_map_["exp_OR"]=exp_template<exp_OR>;
	registered_handler_map_["exp_AND"]=exp_template<exp_AND>;
	registered_handler_map_["exp_BITOR"]=exp_template<exp_BITOR>;
	registered_handler_map_["exp_BITXOR"]=exp_template<exp_BITXOR>;
	registered_handler_map_["exp_BITAND"]=exp_template<exp_BITAND>;
	registered_handler_map_["exp_EQUAL"]=exp_template<exp_EQUAL>;
	registered_handler_map_["exp_UNEQUAL"]=exp_template<exp_UNEQUAL>;
	registered_handler_map_["exp_LESS"]=exp_template<exp_LESS>;
	registered_handler_map_["exp_GREAT"]=exp_template<exp_GREAT>;
	registered_handler_map_["exp_LESS_EQ"]=exp_template<exp_LESS_EQ>;
	registered_handler_map_["exp_GREAT_EQ"]=exp_template<exp_GREAT_EQ>;
	registered_handler_map_["exp_RSHIFT"]=exp_template<exp_RSHIFT>;
	registered_handler_map_["exp_LSHIFT"]=exp_template<exp_LSHIFT>;
	registered_handler_map_["exp_ADD"]=exp_template<exp_ADD>;
	registered_handler_map_["exp_SUB"]=exp_template<exp_SUB>;
	registered_handler_map_["exp_MUL"]=exp_template<exp_MUL>;
	registered_handler_map_["exp_DIV"]=exp_template<exp_DIV>;
	registered_handler_map_["exp_MOD"]=exp_template<exp_MOD>;
	registered_handler_map_["exp_CAST"]=_exp_CAST;

	registered_handler_map_["exp_PRE_INC"]=exp_template<exp_PRE_INC>;
	registered_handler_map_["exp_PRE_DEC"]=exp_template<exp_PRE_DEC>;
	registered_handler_map_["exp_ADDR"]=exp_template<exp_ADDR>;
	registered_handler_map_["exp_DEREF"]=exp_template<exp_DEREF>;
	registered_handler_map_["exp_POS"]=exp_template<exp_POS>;
	registered_handler_map_["exp_NEG"]=exp_template<exp_NEG>;
	registered_handler_map_["exp_COMP"]=exp_template<exp_COMP>;
	registered_handler_map_["exp_NOT"]=exp_template<exp_NOT>;
	registered_handler_map_["exp_SIZE_OF_OBJ"]=exp_template<exp_SIZE_OF_OBJ>;
	registered_handler_map_["exp_SIZE_OF_TYPE"]=_exp_SIZE_OF_TYPE;

	registered_handler_map_["exp_INDEX"]=exp_template<exp_INDEX>;
	registered_handler_map_["exp_CALL"]=_exp_CALL;
	registered_handler_map_["exp_OBJ_MBSL"]=_exp_MBSL<exp_OBJ_MBSL>;
	registered_handler_map_["exp_PTR_MBSL"]=_exp_MBSL<exp_PTR_MBSL>;
	registered_handler_map_["exp_POST_DEC"]=exp_template<exp_POST_DEC>;
	registered_handler_map_["exp_POST_INC"]=exp_template<exp_POST_INC>;

	registered_handler_map_["exp_ID"]=_exp_TERMINAL<exp_ID>;
	registered_handler_map_["exp_STRING"]=_exp_TERMINAL<exp_STRING>;
	registered_handler_map_["exp_CONSTANT_INTEGER"]=_exp_TERMINAL<exp_CONSTANT_INTEGER>;
	registered_handler_map_["exp_CONSTANT_CHARACTER"]=_exp_TERMINAL<exp_CONSTANT_CHARACTER>;
	registered_handler_map_["exp_CONSTANT_FLOATING"]=_exp_TERMINAL<exp_CONSTANT_FLOATING>;
}

