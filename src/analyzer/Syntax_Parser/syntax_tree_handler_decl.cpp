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


struct syntax_node* decl_DECLARATION_TYPE(struct syntax_stack* stack)
{
	struct DECLARATION_TYPE* node=new struct DECLARATION_TYPE();
	LIST_node::off_list(stack->production_node(0),&node->list);
	stack->replace(node);
	return node;
}
struct syntax_node* decl_DECLARATION_DECLARATOR(struct syntax_stack* stack)
{
	struct DECLARATION_DECLARATOR *node=new struct DECLARATION_DECLARATOR;
	LIST_node::off_list(stack->production_node(0),&node->list);
	stack->replace(node);
	return node;
}

struct syntax_node* decl_INIT_DECLARATOR(struct syntax_stack* stack)
{
	struct INIT_DECLARATOR *node=new struct INIT_DECLARATOR();
	node->declaration_declarator_=(struct DECLARATION_DECLARATOR*)stack->production_node(0);
	node->initializer_=stack->top(0);
	stack->replace(node);
	return node;
}
struct syntax_node* decl_INIT_DECLARATOR_partly(struct syntax_stack* stack)
{
	struct INIT_DECLARATOR *node=new struct INIT_DECLARATOR();
	node->declaration_declarator_=(struct DECLARATION_DECLARATOR*)stack->production_node(0);
	node->initializer_=NULL;
	stack->replace(node);
	return node;
}
struct syntax_node* decl_INITIALIZER_expression(struct syntax_stack* stack)
{
	struct INITIALIZER *node=new struct INITIALIZER();
	node->value=stack->production_node(0);
	stack->replace(node);
	return node;
}
struct syntax_node* decl_INITIALIZER_compound(struct syntax_stack* stack)
{
	struct INITIALIZER *node=new struct INITIALIZER();
	node->value=NULL;
	LIST_node::off_list(stack->production_node(1),&node->list);
	stack->replace(node);
	return node;
}

struct syntax_node* decl_PARAMETER_DECLARATION(struct syntax_stack* stack)
{
	struct PARAMETER_DECLARATION *node=new struct PARAMETER_DECLARATION();
	node->declaration_type_=(struct DECLARATION_TYPE*)stack->production_node(0);
	node->declaration_declarator_=(struct DECLARATION_DECLARATOR*)stack->production_node(1);
	stack->replace(node);
	return node;
}
struct syntax_node* decl_PARAMETER_DECLARATION_partly(struct syntax_stack* stack)
{
	struct PARAMETER_DECLARATION *node=new struct PARAMETER_DECLARATION();
	node->declaration_type_=(struct DECLARATION_TYPE*)stack->production_node(0);
	node->declaration_declarator_=NULL;
	stack->replace(node);
	return node;
}
struct syntax_node* decl_DECLARATOR_FUNCTION(struct syntax_stack* stack)
{
	struct DECLARATOR_FUNCTION *node=new struct DECLARATOR_FUNCTION();
	LIST_node::off_list(stack->production_node(1),&node->list);
	stack->replace(node);
	return node;
}
struct syntax_node* decl_DECLARATOR_FUNCTION_partly(struct syntax_stack* stack)
{
	struct DECLARATOR_FUNCTION *node=new struct DECLARATOR_FUNCTION();
	stack->replace(node);
	return node;
}
struct syntax_node* decl_DECLARATOR_ARRAY(struct syntax_stack* stack)
{
	struct DECLARATOR_ARRAY *node=new struct DECLARATOR_ARRAY();
	node->number_of_element=stack->production_node(1);
	stack->replace(node);
	return node;
}
struct syntax_node* decl_DECLARATOR_ARRAY_partly(struct syntax_stack* stack)
{
	struct DECLARATOR_ARRAY *node=new struct DECLARATOR_ARRAY();
	node->number_of_element=NULL;
	stack->replace(node);
	return node;
}
struct syntax_node* decl_DECLARATOR_POINTER(struct syntax_stack* stack)
{
	struct DECLARATOR_POINTER *node=new struct DECLARATOR_POINTER();
	stack->replace(node);
	return node;
}
struct syntax_node* decl_DECLARATOR_POINTER_qualified(struct syntax_stack* stack)
{
	struct DECLARATOR_POINTER *node=new struct DECLARATOR_POINTER();
	LIST_node::off_list(stack->production_node(1),&node->list);
	stack->replace(node);
	return node;
}

struct syntax_node* decl_FUNCTION_DEFINITION(struct syntax_stack* stack)
{
	struct FUNCTION_DEFINITION *node=new struct FUNCTION_DEFINITION();
	node->declaration_type_=(struct DECLARATION_TYPE*)stack->production_node(0);
	node->declaration_declarator_=(struct DECLARATION_DECLARATOR*)stack->production_node(1);
	node->body=stack->top(0);

	node->pos=node->declaration_declarator_->pos;
	stack->replace_dont_set_pos(node);
	return node;
}
struct syntax_node* decl_DECLARATION(struct syntax_stack* stack)
{
	struct DECLARATION *node=new struct DECLARATION();
	node->declaration_type_=(struct DECLARATION_TYPE*)stack->production_node(0);
	LIST_node::off_list(stack->production_node(1),&node->list);

	stack->replace(node);
	return node;
}
struct syntax_node* decl_DECLARATION_partly(struct syntax_stack* stack)
{
	struct DECLARATION *node=new struct DECLARATION();
	node->declaration_type_=(struct DECLARATION_TYPE*)stack->production_node(0);

	stack->replace(node);
	return node;
}

struct syntax_node* decl_TYPE_STRUCT(struct syntax_stack* stack)
{
	struct TYPE_STRUCT *node=new struct TYPE_STRUCT();
	std::string tok;
	std::string extra;
	if(stack->production_node(1)->is_terminal(&tok,&extra) && tok=="ID")
	{
		node->name=extra;
	}
	LIST_node::off_list(stack->top(1),&node->list);
	stack->replace(node);
	return node;
}
struct syntax_node* decl_TYPE_STRUCT_partly(struct syntax_stack* stack)
{
	struct TYPE_STRUCT *node=new struct TYPE_STRUCT();
	std::string tok;
	std::string extra;
	if(stack->production_node(1)->is_terminal(&tok,&extra) && tok=="ID")
	{
		node->name=extra;
	}
	stack->replace(node);
	return node;
}
struct syntax_node* decl_TYPE_UNION(struct syntax_stack* stack)
{
	struct TYPE_UNION *node=new struct TYPE_UNION();
	std::string tok;
	std::string extra;
	if(stack->production_node(1)->is_terminal(&tok,&extra) && tok=="ID")
	{
		node->name=extra;
	}
	LIST_node::off_list(stack->top(1),&node->list);
	stack->replace(node);
	return node;
}
struct syntax_node* decl_TYPE_UNION_partly(struct syntax_stack* stack)
{
	struct TYPE_UNION *node=new struct TYPE_UNION();
	std::string tok;
	std::string extra;
	if(stack->production_node(1)->is_terminal(&tok,&extra) && tok=="ID")
	{
		node->name=extra;
	}
	stack->replace(node);
	return node;
}
struct syntax_node* decl_STRUCT_DECLARATION(struct syntax_stack* stack)
{
	struct STRUCT_DECLARATION *node=new struct STRUCT_DECLARATION();
	node->declaration_type_=(struct DECLARATION_TYPE*)stack->production_node(0);
	LIST_node::off_list(stack->production_node(1),&node->list);
	stack->replace(node);
	return node;
}
struct syntax_node* decl_STRUCT_DECLARATION_partly(struct syntax_stack* stack)
{
	struct STRUCT_DECLARATION *node=new struct STRUCT_DECLARATION();
	node->declaration_type_=(struct DECLARATION_TYPE*)stack->production_node(0);
	stack->replace(node);
	return node;
}
struct syntax_node* decl_STRUCT_DECLARATOR(struct syntax_stack* stack)
{
	struct STRUCT_DECLARATOR *node=new struct STRUCT_DECLARATOR();
	node->declaration_declarator_=(struct DECLARATION_DECLARATOR*)stack->production_node(0);
	node->width=NULL;
	stack->replace(node);
	return node;
}
struct syntax_node* decl_STRUCT_DECLARATOR_field(struct syntax_stack* stack)
{
	struct STRUCT_DECLARATOR *node=new struct STRUCT_DECLARATOR();
	node->declaration_declarator_=(struct DECLARATION_DECLARATOR*)stack->production_node(0);
	node->width=stack->top(0);
	stack->replace(node);
	return node;
}
struct syntax_node* decl_STRUCT_DECLARATOR_field_partly(struct syntax_stack* stack)
{
	struct STRUCT_DECLARATOR *node=new struct STRUCT_DECLARATOR();
	node->declaration_declarator_=NULL;
	node->width=stack->top(0);
	stack->replace(node);
	return node;
}

struct syntax_node* decl_TYPE_ENUM(struct syntax_stack* stack)
{
	struct TYPE_ENUM *node=new struct TYPE_ENUM();
	std::string tok;
	std::string extra;
	if(stack->production_node(1)->is_terminal(&tok,&extra) && tok=="ID")
	{
		node->name=extra;
	}
	LIST_node::off_list(stack->nonterminal(1),&node->list);
	stack->replace(node);
	return node;
}
struct syntax_node* decl_TYPE_ENUM_partly(struct syntax_stack* stack)
{
	struct TYPE_ENUM *node=new struct TYPE_ENUM();
	std::string tok;
	std::string extra;
	if(stack->production_node(1)->is_terminal(&tok,&extra) && tok=="ID")
	{
		node->name=extra;
	}
	stack->replace(node);
	return node;
}
struct syntax_node* decl_ENUMERATOR(struct syntax_stack* stack)
{
	struct ENUMERATOR *node=new struct ENUMERATOR();
	std::string tok;
	std::string extra;
	if(stack->production_node(0)->is_terminal(&tok,&extra) && tok=="ID")
	{
		node->name=extra;
	}else
		assert(0);//never happen.
	node->initializer=NULL;
	stack->replace(node);
	return node;
}
struct syntax_node* decl_ENUMERATOR_initializer(struct syntax_stack* stack)
{
	struct ENUMERATOR *node=new struct ENUMERATOR();
	std::string tok;
	std::string extra;
	if(stack->production_node(0)->is_terminal(&tok,&extra) && tok=="ID")
	{
		node->name=extra;
	}else
		assert(0);//never happen.
	node->initializer=stack->top(0);
	stack->replace(node);
	return node;
}

struct syntax_node* decl_TYPE_NAME_DECLARATION(struct syntax_stack* stack)
{
	struct TYPE_NAME_DECLARATION *node=new struct TYPE_NAME_DECLARATION();
	node->declaration_type_=(struct DECLARATION_TYPE*)stack->production_node(0);
	node->declaration_declarator_=(struct DECLARATION_DECLARATOR*)stack->production_node(1);
	stack->replace(node);
	return node;
}
struct syntax_node* decl_TYPE_NAME_DECLARATION_partly(struct syntax_stack* stack)
{
	struct TYPE_NAME_DECLARATION *node=new struct TYPE_NAME_DECLARATION();
	node->declaration_type_=(struct DECLARATION_TYPE*)stack->production_node(0);
	node->declaration_declarator_=NULL;
	stack->replace(node);
	return node;
}
struct syntax_node* decl_TRANSLATION_UNIT(struct syntax_stack* stack)
{
	struct TRANSLATION_UNIT* node=new struct TRANSLATION_UNIT();
	LIST_node::off_list(stack->production_node(0),&node->list);
	stack->replace(node);
	return node;
}

void syntax_tree_builder::register_handler_decl()
{
	registered_handler_map_["decl_DECLARATION_TYPE"]=decl_DECLARATION_TYPE;
	registered_handler_map_["decl_DECLARATION_DECLARATOR"]=decl_DECLARATION_DECLARATOR;

	registered_handler_map_["decl_INIT_DECLARATOR"]=decl_INIT_DECLARATOR;
	registered_handler_map_["decl_INIT_DECLARATOR_partly"]=decl_INIT_DECLARATOR_partly;
	registered_handler_map_["decl_INITIALIZER_expression"]=decl_INITIALIZER_expression;
	registered_handler_map_["decl_INITIALIZER_compound"]=decl_INITIALIZER_compound;

	registered_handler_map_["decl_PARAMETER_DECLARATION"]=decl_PARAMETER_DECLARATION;
	registered_handler_map_["decl_PARAMETER_DECLARATION_partly"]=decl_PARAMETER_DECLARATION_partly;
	registered_handler_map_["decl_DECLARATOR_FUNCTION"]=decl_DECLARATOR_FUNCTION;
	registered_handler_map_["decl_DECLARATOR_FUNCTION_partly"]=decl_DECLARATOR_FUNCTION_partly;
	registered_handler_map_["decl_DECLARATOR_ARRAY"]=decl_DECLARATOR_ARRAY;
	registered_handler_map_["decl_DECLARATOR_ARRAY_partly"]=decl_DECLARATOR_ARRAY_partly;
	registered_handler_map_["decl_DECLARATOR_POINTER"]=decl_DECLARATOR_POINTER;
	registered_handler_map_["decl_DECLARATOR_POINTER_qualified"]=decl_DECLARATOR_POINTER_qualified;

	registered_handler_map_["decl_TYPE_ENUM"]=decl_TYPE_ENUM;
	registered_handler_map_["decl_TYPE_ENUM_partly"]=decl_TYPE_ENUM_partly;
	registered_handler_map_["decl_ENUMERATOR"]=decl_ENUMERATOR;
	registered_handler_map_["decl_ENUMERATOR_initializer"]=decl_ENUMERATOR_initializer;

	registered_handler_map_["decl_FUNCTION_DEFINITION"]=decl_FUNCTION_DEFINITION;
	registered_handler_map_["decl_DECLARATION"]=decl_DECLARATION;
	registered_handler_map_["decl_DECLARATION_partly"]=decl_DECLARATION_partly;

	registered_handler_map_["decl_TYPE_STRUCT"]=decl_TYPE_STRUCT;
	registered_handler_map_["decl_TYPE_STRUCT_partly"]=decl_TYPE_STRUCT_partly;
	registered_handler_map_["decl_TYPE_UNION"]=decl_TYPE_UNION;
	registered_handler_map_["decl_TYPE_UNION_partly"]=decl_TYPE_UNION_partly;
	registered_handler_map_["decl_STRUCT_DECLARATION"]=decl_STRUCT_DECLARATION;
	registered_handler_map_["decl_STRUCT_DECLARATION"]=decl_STRUCT_DECLARATION;
	registered_handler_map_["decl_STRUCT_DECLARATION_partly"]=decl_STRUCT_DECLARATION_partly;
	registered_handler_map_["decl_STRUCT_DECLARATOR"]=decl_STRUCT_DECLARATOR;
	registered_handler_map_["decl_STRUCT_DECLARATOR_field"]=decl_STRUCT_DECLARATOR_field;
	registered_handler_map_["decl_STRUCT_DECLARATOR_field_partly"]=decl_STRUCT_DECLARATOR_field_partly;

	registered_handler_map_["decl_TYPE_NAME_DECLARATION"]=decl_TYPE_NAME_DECLARATION;
	registered_handler_map_["decl_TYPE_NAME_DECLARATION_partly"]=decl_TYPE_NAME_DECLARATION_partly;

	registered_handler_map_["decl_TRANSLATION_UNIT"]=decl_TRANSLATION_UNIT;
}


