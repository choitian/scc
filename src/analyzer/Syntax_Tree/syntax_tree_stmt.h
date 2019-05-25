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

struct stmt:public syntax_node
{
};
struct stmt_COMPOUND:public stmt
{
	stmt_COMPOUND(){ op="stmt_COMPOUND"; }
	std::vector<struct syntax_node* > decl_list;
	std::vector<struct syntax_node* > stmt_list;
//semantic
	void check();
	void funtion_body_check(struct SCOPE* prototype_scope);
};

struct stmt_LABEL:public stmt
{
	stmt_LABEL(){op="stmt_LABEL";}
	std::string name;
	struct syntax_node *statement;
//semantic
	struct INSTRUCTION_BLOCK *label;
	void check();
};
struct stmt_CASE:public stmt
{
	stmt_CASE(){op="stmt_CASE";}
	struct exp* expression;
	struct syntax_node *statement;
//semantic
	void check();
	struct IC_INS_BLOCK *label;
};
struct stmt_DEFAULT:public stmt
{
	stmt_DEFAULT(){op="stmt_DEFAULT";}
	struct syntax_node *statement;
//semantic
	void check();
	struct IC_INS_BLOCK *label;
};
struct stmt_EXPRESSION:public stmt
{
	stmt_EXPRESSION(){op="stmt_EXPRESSION";}
	struct exp *expression;
//semantic
	void check();
};

struct stmt_IF:public stmt
{
	stmt_IF(){op="stmt_IF";}
	struct exp *expression;
	struct syntax_node *true_statement;
	struct syntax_node *false_statement;
//semantic
	void check();
};
struct stmt_SWITCH:public stmt
{
	stmt_SWITCH(){op="stmt_SWITCH";stmt_default=NULL;}
	struct exp *expression;
	struct syntax_node *statement;
//semantic
	void check();
	bool insert_case(std::string value,struct stmt_CASE* case_);
	struct stmt_DEFAULT* stmt_default;
	std::vector<struct stmt_CASE*> case_list;
	std::map<std::string,struct stmt_CASE*> case_map;
};
struct stmt_WHILE:public stmt
{
	stmt_WHILE(){op="stmt_WHILE";}
	struct exp *expression;
	struct syntax_node *statement;
//semantic
	struct INSTRUCTION_BLOCK* begin;
	struct INSTRUCTION_BLOCK* end;
	void check();
};
struct stmt_DO_WHILE:public stmt
{
	stmt_DO_WHILE(){op="stmt_DO_WHILE";}
	struct exp *expression;
	struct syntax_node *statement;
//semantic
	void check();
};
struct stmt_FOR:public stmt
{
	stmt_FOR(){op="stmt_FOR";}
	struct exp *initialization_expression;
	struct exp *controlling_expression;
	struct exp *reinitialization_expression;
	struct syntax_node *statement;
//semantic
	void check();
};
struct stmt_GOTO:public stmt
{
	stmt_GOTO(){op="stmt_GOTO";}
	std::string name;
//semantic
	struct iLABEL *label;
	void check();
};
struct stmt_CONTINUE:public stmt
{
	stmt_CONTINUE(){op="stmt_CONTINUE";}
//semantic
	void check();
	struct syntax_node* outter_loop;
};
struct stmt_BREAK:public stmt
{
	stmt_BREAK(){op="stmt_BREAK";}
//semantic
	void check();
	struct syntax_node* outter_loop_or_switch;
};
struct stmt_RETURN:public stmt
{
	stmt_RETURN(){op="stmt_RETURN";}
	struct exp *expression;
//semantic
	void check();
};


