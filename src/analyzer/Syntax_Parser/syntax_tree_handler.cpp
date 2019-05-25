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

struct syntax_node* on___DEFAULT__(struct syntax_stack* stack)
{
	struct syntax_node* node;
	node=new TERMINAL_node("Undefined","");
	stack->replace(node);
	return node;
}
struct syntax_node* on___NULL__(struct syntax_stack* stack)
{
	struct syntax_node* node;
	node=new TERMINAL_node("__NULL__","");
	stack->push(node);
	return node;
}

struct syntax_node* on_SKIP(struct syntax_stack* stack)
{
	return NULL;
}
struct syntax_node* on_REUSE_1st(struct syntax_stack* stack)
{
	stack->replace(stack->production_node(0));
	return NULL;
}
struct syntax_node* on_REUSE_2nd(struct syntax_stack* stack)
{
	stack->replace(stack->production_node(1));
	return NULL;
}

struct syntax_node* on_LIST(struct syntax_stack* stack)
{
		struct LIST_node *ln;
		ln=new struct LIST_node(stack->production_node(0),stack->production_node(stack->current_production()->len-1));

		stack->replace(ln);
		return ln;

}
struct syntax_node* on_LIST_reverse(struct syntax_stack* stack)
{
		struct LIST_node *ln;
		ln=new struct LIST_node(stack->production_node(stack->current_production()->len-1),stack->production_node(0));

		stack->replace(ln);
		return ln;

}
struct syntax_node* on_LIST_triple(struct syntax_stack* stack)
{
		struct LIST_node *ln;
		ln=new struct LIST_node(stack->production_node(0),stack->production_node(1));
		ln=new struct LIST_node(ln,stack->production_node(2));
		stack->replace(ln);
		return ln;

}

struct syntax_node* ENTER_SCOPE(struct syntax_stack* stack)
{
	parese_scope::enter_scope();
	return on___NULL__(stack);
}
struct syntax_node* EXIT_SCOPE(struct syntax_stack* stack)
{
	parese_scope::exit_scope();
	return on___NULL__(stack);
}
static bool declaration_specifiers_has_typedef(struct DECLARATION_TYPE *declaration_specifiers)
{
	for(size_t i=0;i<declaration_specifiers->list.size();i++)
	{
		if("TERMINAL"==declaration_specifiers->list.at(i)->op)
		{
			struct TERMINAL_node *terminal=static_cast<struct TERMINAL_node *>(declaration_specifiers->list.at(i));
			if(terminal->extra=="typedef")
				return true;
		}
	}
	return false;
}
static bool declaration_declarator_has_name(struct DECLARATION_DECLARATOR *declarator,std::string *name)
{
	for(size_t i=0;i<declarator->list.size();i++)
	{
		if("TERMINAL"==declarator->list.at(i)->op)
		{
			struct TERMINAL_node *terminal=static_cast<struct TERMINAL_node *>(declarator->list.at(i));
			if(terminal->tok=="ID")
			{
				if(name!=NULL)
					*name=terminal->extra;
				return true;
			}
		}
	}
	return false;
}
static bool is_declarator_function(struct DECLARATION_DECLARATOR *declarator,struct DECLARATOR_FUNCTION** declarator_function)
{
	bool is_function=false;
	std::string tok,extra;
	struct syntax_node* node;
	for(size_t i=0;i<declarator->list.size();i++)
	{
		node=declarator->list.at(i);
		if(node->is_terminal(&tok,&extra))
		{
			if(tok=="ID")
			{
			}else if(tok=="cdecl")
			{
			}else if(tok=="stdcall")
			{
			}else
				assert(0);
		}else if(node->op=="DECLARATOR_POINTER")
		{
			is_function=false;
		}else if(node->op=="DECLARATOR_ARRAY")
		{
			is_function=false;
		}else if(node->op=="DECLARATOR_FUNCTION")
		{
			*declarator_function=static_cast<struct DECLARATOR_FUNCTION*>(node);
			is_function=true;
		}else
			assert(0);
	}
	return is_function;
}
struct syntax_node* FUNCTION_ENTER_SCOPE(struct syntax_stack* stack)
{
	struct DECLARATION_DECLARATOR *function_declarator;
	function_declarator=(struct DECLARATION_DECLARATOR *)stack->top(0);
	std::string name;
	if(declaration_declarator_has_name(function_declarator,&name))
		parese_scope::current->put(name,"variable");

	struct DECLARATOR_FUNCTION* df;
	if(is_declarator_function(function_declarator,&df))
	{
		parese_scope::enter_scope();
		for(size_t i=0;i<df->list.size();i++)
		{
			if(df->list.at(i)->op=="PARAMETER_DECLARATION")
			{
				struct PARAMETER_DECLARATION* pd;
				std::string name;
				pd=(struct PARAMETER_DECLARATION*)df->list.at(i);
				if(pd->declaration_declarator_!=NULL && declaration_declarator_has_name(pd->declaration_declarator_,&name))
					parese_scope::current->put(name,"variable");
			}
		}
	}

	return on___NULL__(stack);
}
struct syntax_node* REGISTER_TYPE_DEF_NAME(struct syntax_stack* stack)
{
	struct DECLARATION_TYPE *declaration_specifiers;

	declaration_specifiers=(struct DECLARATION_TYPE *)stack->top(1);
	std::string type_,name;
	if(declaration_specifiers_has_typedef(declaration_specifiers))
		type_="typedef_name";
	else
		type_="variable";
	std::vector<struct syntax_node*> list;
	LIST_node::off_list(stack->top(0),&list);
	struct DECLARATION_DECLARATOR *dds;
	for(std::vector<struct syntax_node*>::iterator it=list.begin();it!=list.end();it++)
	{
		dds=((struct INIT_DECLARATOR *)(*it))->declaration_declarator_;
		if(declaration_declarator_has_name(dds,&name))
			parese_scope::current->put(name,type_);
	}
	return on___NULL__(stack);
}
void syntax_tree_builder::register_handler_typedef()
{
	registered_handler_map_["ENTER_SCOPE"]=ENTER_SCOPE;
	registered_handler_map_["EXIT_SCOPE"]=EXIT_SCOPE;
	registered_handler_map_["FUNCTION_ENTER_SCOPE"]=FUNCTION_ENTER_SCOPE;
	registered_handler_map_["REGISTER_TYPE_DEF_NAME"]=REGISTER_TYPE_DEF_NAME;
}
void syntax_tree_builder::register_handler()
{
	registered_handler_map_["__DEFAULT__"]=on___DEFAULT__;
	registered_handler_map_["__NULL__"]=on___NULL__;

	registered_handler_map_["SKIP"]=on_SKIP;
	registered_handler_map_["REUSE_1st"]=on_REUSE_1st;
	registered_handler_map_["REUSE_2nd"]=on_REUSE_2nd;
	registered_handler_map_["LIST"]=on_LIST;
	registered_handler_map_["LIST_reverse"]=on_LIST_reverse;
	registered_handler_map_["LIST_triple"]=on_LIST_triple;

	register_handler_typedef();
	register_handler_decl();
	register_handler_stmt();
	register_handler_exp();
}

