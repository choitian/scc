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

struct Token* syntax_stack::current_token()
{
	return parser_state_->lookahead;
}
struct LALR_Production*	syntax_stack::current_production()
{
	return LALR_Production::id_to_LALR_Production[parser_state_->action->u.production];
}
void syntax_stack::replace_dont_set_pos(struct syntax_node* sn)
{
	track_.resize(track_.size()-this->current_production()->len);
	track_.push_back(sn);
}
void syntax_stack::replace(struct syntax_node* sn)
{
	track_.resize(track_.size()-this->current_production()->len);
	if(sn!=NULL)
		sn->pos=this->parser_state_->POSITION();
	track_.push_back(sn);
}
void syntax_stack::push(struct syntax_node* sn)
{
	if(sn!=NULL)
		sn->pos=this->parser_state_->POSITION();
	track_.push_back(sn);
}
struct syntax_node* syntax_stack::nonterminal(size_t index)
{
	std::vector<int >* nonterminal_list=&(current_production()->nonterminal_list);
	int nonterminal_index;
	if(index >= nonterminal_list->size())
		return NULL;
	nonterminal_index=nonterminal_list->at(index);
	return production_node(nonterminal_index);
}
struct syntax_node* syntax_stack::production_node(size_t index)
{
	if(index >= current_production()->len)
		return NULL;
	if(current_production()->len > track_.size())
		return NULL;
	return track_.at(track_.size()-current_production()->len + index);
}
struct syntax_node* syntax_stack::top(size_t index)
{
	if(index >= track_.size())
		return NULL;
	return track_.at(track_.size()-1-index);
}

void syntax_tree_builder::terminal(struct syntax_stack* stack)
{
	struct TERMINAL_node* node;
	std::string tok=stack->current_token()->tok;
	std::string extra=stack->current_token()->extra;
	node=new struct TERMINAL_node(tok,extra);
	if(tok=="LPAREN" || tok=="LBRACE" || tok=="SEMICOLON" || tok=="COMMA" )
	{
		parese_scope::typedef_name_trigger="on";
	}
	if(tok=="void" || tok=="char" || tok=="short" || tok=="int" ||
		tok=="long" || tok=="float" || tok=="double" || tok=="signed" || tok=="unsigned" ||
		tok=="struct" || tok=="union" || tok=="enum" || tok=="typedef_name")
	{
		parese_scope::typedef_name_trigger="off";
	}
	stack->push(node);
}
void syntax_tree_builder::production(struct syntax_stack* stack)
{
	std::string op=stack->current_production()->action_op;
	if(registered_handler_map_.count(op)!=0)
	{
		registered_handler_map_[op](stack);
	}else
	{
		registered_handler_map_["__DEFAULT__"](stack);
	}
}

int syntax_tree_builder::BEGIN()
{
	parese_scope::enter_file_scope();
	return 0;
}
struct TRANSLATION_UNIT* syntax_tree_builder::END()
{
	if(this->stack_->track_.size()==1 && this->stack_->track_.back()->op=="TRANSLATION_UNIT")
		return static_cast<struct TRANSLATION_UNIT*>(this->stack_->track_.back());
	return NULL;
}
int syntax_tree_builder::on_ERROR()
{
	::SCC_SYNTAX_ERROR(stack_->parser_state_->POSITION(),"unexpected %s",stack_->parser_state_->lookahead->toString().c_str());
	stack_->parser_state_->error_num++;
	if(stack_->parser_state_->lookahead->tok=="__END__")
		return -1;
	return 0;
}
int syntax_tree_builder::on_SHIFT()
{
	SCC_DEBUG("SHIFT: %s",stack_->parser_state_->lookahead->extra.c_str());
	terminal(stack_);
	return 0;
}
int syntax_tree_builder::on_REDUCE()
{
	SCC_DEBUG("REDUCE TO: %s",LALR_Production::production_head_name(stack_->parser_state_->action->u.production).c_str());
	production(stack_);
	return 0;
}
int syntax_tree_builder::on_ACCEPT()
{
	if(stack_->parser_state_->error_num==0)
		::SCC_MSG("SYNTAX ACCEPT.");
	else
		::SCC_MSG("SYNTAX NONACCEPT.");
	return 0;
}
struct Token * syntax_tree_builder::typedef_name_alter(struct Token *tok)
{
	if(tok->tok=="ID")
	{
		if(parese_scope::current->get(tok->extra)=="typedef_name" && parese_scope::typedef_name_trigger=="on")
		{
			tok->tok="typedef_name";
		}
	}
	return tok;
}
void syntax_tree_builder::set_parse_state(struct Parser_State *state)
{
	this->stack_->parser_state_=state;
}



