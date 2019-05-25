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

#include "Syntax_Parser .h"
#include "..\Syntax_Tree\syntax_tree.h"
#include "..\Syntax_Tree\syntax_tree_decl.h"
#include "..\Syntax_Tree\syntax_tree_stmt.h"
#include "..\Syntax_Tree\syntax_tree_exp.h"

struct LIST_node:public syntax_node
{
	struct syntax_node *first_,*second_;
	std::list<struct syntax_node *> list;

	LIST_node(struct syntax_node *first,struct syntax_node* second)
	{
		op="LIST";
		first_= first;
		second_=second;
	}
static void off_list(struct syntax_node *node,std::vector<struct syntax_node* >* result)
{
	if(node==NULL)
		return;
	if(node->op=="LIST")
	{
		struct LIST_node* list=static_cast<struct LIST_node *>(node);
		off_list(list->first_,result);
		off_list(list->second_,result);
	}
	else
		result->push_back(node);
}
};
struct syntax_stack
{
	struct Parser_State *parser_state_;
	std::vector<struct syntax_node*> track_;

	struct Token*	current_token();
	struct LALR_Production*	current_production();

	void push(struct syntax_node* sn);
	void replace_dont_set_pos(struct syntax_node* sn);
	void replace(struct syntax_node* sn);
	struct syntax_node* production_node(size_t index);
	struct syntax_node* nonterminal(size_t index);
	struct syntax_node* top(size_t index);
};
typedef struct syntax_node* (*HANDLER)(struct syntax_stack* stack);
struct syntax_tree_builder
{

	void terminal(struct syntax_stack* stack);
	void production(struct syntax_stack* stack);

	syntax_tree_builder()
	{ 
		this->stack_=new struct syntax_stack();
		register_handler(); 
	}

	int BEGIN();
	struct TRANSLATION_UNIT* END();
	int on_ERROR();
	int on_SHIFT();
	int on_REDUCE();
	int on_ACCEPT();
	struct Token *typedef_name_alter(struct Token *tok);
	struct syntax_stack *stack_;
	void set_parse_state(struct Parser_State *state);
private:
	std::map<std::string,HANDLER> registered_handler_map_;
	void register_handler();
	void register_handler_exp();
	void register_handler_stmt();
	void register_handler_decl();
	void register_handler_typedef();
};

