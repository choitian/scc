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

struct syntax_node* _stmt_COMPOUND_empty(struct syntax_stack* stack)
{
	struct stmt_COMPOUND* node=new struct stmt_COMPOUND();
	stack->replace(node);
	return node;
}
struct syntax_node* _stmt_COMPOUND_decl(struct syntax_stack* stack)
{
	struct stmt_COMPOUND* node=new struct stmt_COMPOUND();
	LIST_node::off_list(stack->production_node(1),&node->decl_list);
	stack->replace(node);
	return node;
}
struct syntax_node* _stmt_COMPOUND_stmt(struct syntax_stack* stack)
{
	struct stmt_COMPOUND* node=new struct stmt_COMPOUND();
	LIST_node::off_list(stack->production_node(1),&node->stmt_list);
	stack->replace(node);
	return node;
}
struct syntax_node* _stmt_COMPOUND(struct syntax_stack* stack)
{
	struct stmt_COMPOUND* node=new struct stmt_COMPOUND();
	LIST_node::off_list(stack->production_node(1),&node->decl_list);
	LIST_node::off_list(stack->production_node(2),&node->stmt_list);
	stack->replace(node);
	return node;
}

struct syntax_node* _stmt_LABEL(struct syntax_stack* stack)
{
	struct stmt_LABEL* node=new struct stmt_LABEL();
	std::string tok;
	std::string extra;
	if(stack->production_node(0)->is_terminal(&tok,&extra) && tok=="ID")
	{
		node->name=extra;
	}
	node->statement=stack->top(0);
	stack->replace(node);
	return node;
}
struct syntax_node* _stmt_CASE(struct syntax_stack* stack)
{
	struct stmt_CASE* node=new struct stmt_CASE();
	node->expression=static_cast<struct exp*>(stack->production_node(1));
	node->statement=stack->top(0);

	node->pos=stack->production_node(0)->pos;
	stack->replace_dont_set_pos(node);
	return node;
}
struct syntax_node* _stmt_DEFAULT(struct syntax_stack* stack)
{
	struct stmt_DEFAULT* node=new struct stmt_DEFAULT();
	node->statement=stack->top(0);

	node->pos=stack->production_node(0)->pos;
	stack->replace_dont_set_pos(node);
	return node;
}

struct syntax_node* _stmt_EXPRESSION(struct syntax_stack* stack)
{
	struct stmt_EXPRESSION* node=new struct stmt_EXPRESSION();
	node->expression=static_cast<struct exp*>(stack->production_node(0));
	stack->replace(node);
	return node;
}
struct syntax_node* _stmt_EXPRESSION_empty(struct syntax_stack* stack)
{
	struct stmt_EXPRESSION* node=new struct stmt_EXPRESSION();
	node->expression=NULL;
	stack->replace(node);
	return node;
}

struct syntax_node* _stmt_IF(struct syntax_stack* stack)
{
	struct stmt_IF* node=new struct stmt_IF();
	node->expression=static_cast<struct exp*>(stack->production_node(2));
	node->true_statement=stack->top(0);
	node->false_statement=NULL;
	stack->replace(node);
	return node;
}
struct syntax_node* _stmt_IF_else(struct syntax_stack* stack)
{
	struct stmt_IF* node=new struct stmt_IF();
	node->expression=static_cast<struct exp*>(stack->production_node(2));
	node->true_statement=stack->production_node(4);
	node->false_statement=stack->top(0);
	stack->replace(node);
	return node;
}
struct syntax_node* _stmt_SWITCH(struct syntax_stack* stack)
{
	struct stmt_SWITCH* node=new struct stmt_SWITCH();
	node->expression=static_cast<struct exp*>(stack->production_node(2));
	node->statement=stack->top(0);
	stack->replace(node);
	return node;
}

struct syntax_node* _stmt_WHILE(struct syntax_stack* stack)
{
	struct stmt_WHILE* node=new struct stmt_WHILE();
	node->expression=static_cast<struct exp*>(stack->production_node(2));
	node->statement=stack->top(0);
	stack->replace(node);
	return node;
}
struct syntax_node* _stmt_DO_WHILE(struct syntax_stack* stack)
{
	struct stmt_DO_WHILE* node=new struct stmt_DO_WHILE();
	node->statement=stack->production_node(1);
	node->expression=static_cast<struct exp*>(stack->production_node(4));
	stack->replace(node);
	return node;
}
struct syntax_node* _stmt_FOR(struct syntax_stack* stack)
{
	struct stmt_FOR* node=new struct stmt_FOR();
	node->initialization_expression=static_cast<struct exp*>(stack->production_node(2));
	if(node->initialization_expression->is_null())
	{
		node->initialization_expression=NULL;
	}
	node->controlling_expression=static_cast<struct exp*>(stack->production_node(4));
	if(node->controlling_expression->is_null())
	{
		node->controlling_expression=NULL;
	}
	node->reinitialization_expression=static_cast<struct exp*>(stack->production_node(6));
	if(node->reinitialization_expression->is_null())
	{
		node->reinitialization_expression=NULL;
	}

	node->statement=stack->top(0);
	stack->replace(node);
	return node;
}

struct syntax_node* _stmt_GOTO(struct syntax_stack* stack)
{
	struct stmt_GOTO* node=new struct stmt_GOTO();
	std::string tok;
	std::string extra;
	if(stack->production_node(1)->is_terminal(&tok,&extra) && tok=="ID")
	{
		node->name=extra;
	}
	stack->replace(node);
	return node;
}
struct syntax_node* _stmt_CONTINUE(struct syntax_stack* stack)
{
	struct stmt_CONTINUE* node=new struct stmt_CONTINUE();
	stack->replace(node);
	return node;
}
struct syntax_node* _stmt_BREAK(struct syntax_stack* stack)
{
	struct stmt_BREAK* node=new struct stmt_BREAK();
	stack->replace(node);
	return node;
}
struct syntax_node* _stmt_RETURN(struct syntax_stack* stack)
{
	struct stmt_RETURN* node=new struct stmt_RETURN();
	node->expression=static_cast<struct exp*>(stack->production_node(1));
	stack->replace(node);
	return node;
}
struct syntax_node* _stmt_RETURN_void(struct syntax_stack* stack)
{
	struct stmt_RETURN* node=new struct stmt_RETURN();
	node->expression=NULL;
	stack->replace(node);
	return node;
}
void syntax_tree_builder::register_handler_stmt()
{
	registered_handler_map_["stmt_COMPOUND_empty"]=_stmt_COMPOUND_empty;
	registered_handler_map_["stmt_COMPOUND_decl"]=_stmt_COMPOUND_decl;
	registered_handler_map_["stmt_COMPOUND_stmt"]=_stmt_COMPOUND_stmt;
	registered_handler_map_["stmt_COMPOUND"]=_stmt_COMPOUND;

	registered_handler_map_["stmt_LABEL"]=_stmt_LABEL;
	registered_handler_map_["stmt_CASE"]=_stmt_CASE;
	registered_handler_map_["stmt_DEFAULT"]=_stmt_DEFAULT;

	registered_handler_map_["stmt_EXPRESSION"]=_stmt_EXPRESSION;
	registered_handler_map_["stmt_EXPRESSION_empty"]=_stmt_EXPRESSION_empty;

	registered_handler_map_["stmt_IF"]=_stmt_IF;
	registered_handler_map_["stmt_IF_else"]=_stmt_IF_else;
	registered_handler_map_["stmt_SWITCH"]=_stmt_SWITCH;

	registered_handler_map_["stmt_WHILE"]=_stmt_WHILE;
	registered_handler_map_["stmt_DO_WHILE"]=_stmt_DO_WHILE;
	registered_handler_map_["stmt_FOR"]=_stmt_FOR;

	registered_handler_map_["stmt_GOTO"]=_stmt_GOTO;
	registered_handler_map_["stmt_CONTINUE"]=_stmt_CONTINUE;
	registered_handler_map_["stmt_BREAK"]=_stmt_BREAK;
	registered_handler_map_["stmt_RETURN"]=_stmt_RETURN;
	registered_handler_map_["stmt_RETURN_void"]=_stmt_RETURN_void;
}

