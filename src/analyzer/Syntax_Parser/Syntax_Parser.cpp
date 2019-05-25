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
#include "Syntax_Parser .h"
#include "syntax_tree_builder.h"

struct Parser_State* current_state;
void build_syntax_tree_token(struct Parser_State* parser_state);
void build_syntax_tree(struct Parser_State* parser_state);
std::vector<struct DFA_State* > DFA_State::DFA_State_vector;

std::map<int,struct LALR_Symbol* > LALR_Symbol::id_to_LALR_Symbol;
std::map<int,struct LALR_Production* > LALR_Production::id_to_LALR_Production;
std::map<int,struct LALR_State* > LALR_State::id_to_LALR_State;

namespace
{
	int get_integer(char** buffer,char end)
	{
		int i;
		i=atoi(*buffer);
		while(**buffer!=end)
		{
			(*buffer)++;
		}
		return i;
	}
	std::string get_identifier(char** buffer)
	{
		char* start=*buffer;
		std::string id_string;
		if(IsLetter(**buffer))
		{
			while(IsLetter(**buffer) || IsDigit(**buffer))
			{
				(*buffer)++;
			}
		}
		id_string.append(start,*buffer-start);
		return id_string;
	}
	std::string get_untill(char** buffer,char endmarker)
	{
		char* start=*buffer;
		std::string txt;
		while(**buffer!=endmarker)
		{
			(*buffer)++;
		}
		txt.append(start,*buffer-start);
		return txt;
	}
	void pass(char** buffer,char ch)
	{
		if(**buffer!=ch)
		{
			::SCC_ERROR("Precompile","expect %c\n",ch);
		}
		(*buffer)++;
	}
	bool get_next_number(std::string::iterator begin,std::string::iterator end,
		std::string::iterator *suffix,int*result)
	{
		std::string::iterator cur=begin;
		while(cur!=end)
		{
			std::string number;
			//to Digit
			while( cur!=end && !IsDigit(*cur))
			{
				cur++;
			}
			//get Digit
			while( cur!=end && IsDigit(*cur))
			{
				number.append(1,*cur);
				cur++;
			}
			if(!number.empty())
			{
				if(result!=NULL)
					*result=string_to_int(number);
				if(suffix!=NULL)
					*suffix=cur;
				return true;
			}
		}
		return false;
	}
	bool get_next_identiier(std::string::iterator begin,std::string::iterator end,
		std::string::iterator *suffix,std::string *result)
	{
		std::string::iterator cur=begin;
		while(cur!=end)
		{
			std::string identiier;
			//to letter
			while(cur!=end && !IsLetter(*cur))
			{
				cur++;
			}
			//get Letter or Digit
			while( cur!=end && (IsLetter(*cur) ||IsDigit(*cur)) )
			{
				identiier.append(1,*cur);
				cur++;
			}
			if(!identiier.empty())
			{
				if(result!=NULL)
					*result=identiier;
				if(suffix!=NULL)
					*suffix=cur;
				return true;
			}
		}
		return false;
	}
	void decode_nonterminal_text(std::string nonterminal_text,std::vector<int >* nonterminal_list)
	{
		int index;
		std::string::iterator suffix,end;
		suffix=nonterminal_text.begin();
		end=nonterminal_text.end();

		while(get_next_number(suffix,end,&suffix,&index))
		{
			nonterminal_list->push_back(index);
		}
	}
	void decode_action(std::string action,std::string *op,std::vector<int >* arg_list)
	{
		std::string::iterator suffix,end;
		suffix=action.begin();
		end=action.end();

		get_next_identiier(suffix,end,&suffix,op);

		int index;
		while(get_next_number(suffix,end,&suffix,&index))
		{
			arg_list->push_back(index);
		}
	}
}
struct parese_scope* parese_scope::global=NULL;
struct parese_scope* parese_scope::current=NULL;
std::string parese_scope::typedef_name_trigger="on";

void Syntax_Parser::setup_DFA_table()
{
	char *cur,*cur_base,*cur_end;
	int size;
	char index;
	cur_base=file_to_char_array(SCC_ENV.DFA_table,&size);
	cur_end=cur_base+size-1;
	cur=cur_base;

	struct DFA_State* state;
	while(cur <= cur_end)
	{
		state=DFA_State::new_();
		state->id=get_integer(&cur,':');
		pass(&cur,':');
		if(cur[0]!=':')
		{
			state->accept=1;
			state->tok=get_identifier(&cur);
		}
		pass(&cur,':');
		while(cur[0]=='[')
		{
			pass(&cur,'[');
			index=cur[0];
			cur++;
			pass(&cur,',');
			state->go_to[index]=get_integer(&cur,']');
			pass(&cur,']');
		}
		pass(&cur,';');
		pass(&cur,'\n');
		DFA_State::DFA_State_vector.push_back(state);
	}
}
struct Token* Syntax_Parser::get_next_token_()
{
	struct Token* tok;
	struct DFA_State *state;
	char *start;
	char current;
	std::stack<struct DFA_State *> state_track;

	tok=Token::new_();
	if(PState.cur > PState.end)
	{
		tok->tok="__END__";
		return tok;
	}

	state=DFA_State::of_(1);
	state_track.push(state);
	start=PState.cur;
	current=PState.cur[0];
	while(1)
	{
		if(state->go_to.count(current)==0)
			break;
		else
		{
			PState.cur++;
			state=DFA_State::of_(state->go_to[current]);
			state_track.push(state);
			current=PState.cur[0];
		}
		if(state->accept!=0 && state->tok=="MULTILINE_COMMENT")//for MULTILINE_COMMENT,MIN length rule,not MAX length rule
			break;
	}
	while(!state_track.empty())
	{
		state=state_track.top();
		state_track.pop();
		if(state->accept!=0)
		{
			tok->tok=state->tok;
			tok->extra.assign(start,PState.cur-start);
			return tok;
		}
		PState.cur--;
	}
	tok->tok="ERROR";
	return tok;
}
struct Token *Syntax_Parser::get_next_token()
{
	struct Token *tok;
	tok=get_next_token_();
	if(tok->tok=="NEWLINE")
	{
		PState.inc_line();
		return get_next_token();
	}else if(tok->tok=="COMMENT"|| tok->tok=="MULTILINE_COMMENT")
	{
		std::string comment=tok->extra;
		for(size_t i=0;i<comment.size();i++)
		{
			if(comment[i]=='\n')
				PState.inc_line();
		}
		return get_next_token();
	}else if(tok->tok=="SKIP")
	{
		return get_next_token();
	}else if(tok->tok=="_ERROR")
	{
		::SCC_SYNTAX_ERROR(PState.POSITION(),"unexpect character.");
		PState.cur++;
		return get_next_token();
	}
	return tok;
}
struct Token *Syntax_Parser::get_next_parse_token()
{
	struct Token *tok;
	tok=get_next_token();
	return builder->typedef_name_alter(tok);
}
void Syntax_Parser::setup_LALR_table()
{

	char *cur,*cur_base,*cur_end;
	int size;
	cur_base=file_to_char_array(SCC_ENV.LALR_table,&size);
	cur_end=cur_base+size-1;
	cur=cur_base;

	{
		struct LALR_Symbol* symbol;

		while(!(cur[0]=='%' && cur[1]=='%'))
		{
			symbol=LALR_Symbol::new_();
			symbol->id=get_integer(&cur,':');
			pass(&cur,':');
			symbol->name=get_identifier(&cur);
			pass(&cur,';');
			pass(&cur,'\n');

			LALR_Symbol::id_to_LALR_Symbol[symbol->id]=symbol;
		}
	}
	pass(&cur,'%');
	pass(&cur,'%');
	pass(&cur,'\n');
	{
		struct LALR_Production* production;
		while(!(cur[0]=='%' && cur[1]=='%'))
		{
			production=LALR_Production::new_();
			production->id=get_integer(&cur,':');
			pass(&cur,':');
			production->len=get_integer(&cur,':');
			pass(&cur,':');
			production->head=get_integer(&cur,':');
			pass(&cur,':');
			production->action=get_untill(&cur,'[');
			if(!production->action.empty())
				decode_action(production->action,&production->action_op,&production->arg_list);
			production->nonterminal_text=get_untill(&cur,';');
			if(!production->action.empty())
				decode_nonterminal_text(production->nonterminal_text,&production->nonterminal_list);
			pass(&cur,';');
			pass(&cur,'\n');

			LALR_Production::id_to_LALR_Production[production->id]=production;
		}
	}
	pass(&cur,'%');
	pass(&cur,'%');
	pass(&cur,'\n');
	{
		struct LALR_State* state;
		struct LALR_Action* action;
		while(!(cur[0]=='%' && cur[1]=='%'))
		{
			state=LALR_State::new_();

			state->id=get_integer(&cur,':');
			pass(&cur,':');
			while(*cur=='[')
			{
				action=LALR_Action::new_();
				pass(&cur,'[');
				action->lookahead=get_integer(&cur,',');
				state->action_map[LALR_Symbol::id_to_LALR_Symbol[action->lookahead]->name]=action;
				pass(&cur,',');
				action->type=get_integer(&cur,',');
				pass(&cur,',');
				action->u.action_extra=get_integer(&cur,']');
				pass(&cur,']');
			}
			pass(&cur,';');
			pass(&cur,'\n');
			LALR_State::id_to_LALR_State[state->id]=state;
		}
	}
}
void Syntax_Parser::setup_builder(struct syntax_tree_builder* bd)
{
	builder=bd;
	bd->set_parse_state(&PState);
}
struct TRANSLATION_UNIT* Syntax_Parser::syntax_parsing()
{
	this->PState.setup_input(SCC_ENV.input_file);

//go parse!
	builder->BEGIN();
	PState.lookahead=get_next_parse_token();
	PState.track.push_back(LALR_State::Start_State());
	while(1)
	{
		PState.action=PState.track.back()->action_of(PState.lookahead->tok);
		if(PState.action==NULL)//no move!
		{
			if(builder->on_ERROR()==-1)
				break;
			else
				PState.lookahead=this->get_next_parse_token();
			continue;
		}
		if(PState.action->type==Action_TYPE::SHIFT_)
		{
			builder->on_SHIFT();

			PState.track.push_back(LALR_State::id_to_LALR_State[PState.action->u.go_to]);
			PState.lookahead=get_next_parse_token();
		}else if(PState.action->type==Action_TYPE::REDUCE_)
		{
			builder->on_REDUCE();

			int new_size;
			int production_len;
			production_len=LALR_Production::production_len(PState.action->u.production);
			new_size=PState.track.size();
			new_size=new_size-production_len;
			PState.track.resize(new_size);

			int go_to;
			struct LALR_Action* action;
			std::string production_head_name;
			production_head_name=LALR_Production::production_head_name(PState.action->u.production);
			action=PState.track.back()->action_of(production_head_name);
			go_to=action->u.go_to;
			PState.track.push_back(LALR_State::id_to_LALR_State[go_to]);

		}else if(PState.action->type==Action_TYPE::ACCEPT_)
		{
			builder->on_ACCEPT();
			break;
		}
	}
	return builder->END();
}

Syntax_Parser::Syntax_Parser()
{
	this->setup_DFA_table();
	this->setup_LALR_table();
	struct syntax_tree_builder *bd=new  struct syntax_tree_builder();
	this->setup_builder(bd);
}
void Syntax_Parser::TEST()
{
}
