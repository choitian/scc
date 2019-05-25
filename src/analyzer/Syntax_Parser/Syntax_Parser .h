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

#include "..\lib\common.h"

enum Action_TYPE{
	SHIFT_,REDUCE_,GOTO_,ACCEPT_,ERROR_
};
struct DFA_State
{
	static 	std::vector<struct DFA_State* > DFA_State_vector;
	int id;
	int accept;
	std::string tok;
	std::map<char,int> go_to;
static struct DFA_State* new_()
{
	struct DFA_State* s=new struct DFA_State;
	s->accept=0;
	s->id=0;
	return s;
}
static struct DFA_State* of_(int id)
{
	return DFA_State_vector[id-1];
}
};
struct Token
{
	std::string tok;
	std::string extra;
static 	struct Token* new_()
{
	return new struct Token;
}
	std::string toString()
	{
		std::string text;
		text.append(tok);
		text.append("[");
		text.append(extra);
		text.append("]");
		return text;
	}
};

struct LALR_Action
{
	int lookahead;
	int type;
	union{
		int action_extra;
		int go_to;
		int production;
	}u;
static struct LALR_Action* new_()
{
	return new struct LALR_Action;
}
};
struct LALR_Symbol
{
	static std::map<int,struct LALR_Symbol* > id_to_LALR_Symbol;

	int id;
	std::string name;
static struct LALR_Symbol* new_()
{
	return new struct LALR_Symbol;
}
};
struct LALR_Production
{
	static std::map<int,struct LALR_Production* > id_to_LALR_Production;

	int id;
	size_t len;
	int head;
	std::string action;
	std::string action_op;
	std::vector<int > arg_list;
	std::string nonterminal_text;
	std::vector<int > nonterminal_list;
static struct LALR_Production* new_()
{
	return new struct LALR_Production;
}
static int production_len(int production)
{
	if(id_to_LALR_Production.count(production)!=0)
	{
		return id_to_LALR_Production[production]->len;
	}
	return -1;
}
static std::string production_action(int production)
{
	std::string action;
	if(id_to_LALR_Production.count(production)!=0)
	{
		action=id_to_LALR_Production[production]->action;
	}
	return action;
}
static std::string production_head_name(int production)
{
	std::string name;
	if(id_to_LALR_Production.count(production)!=0)
	{
		int head=id_to_LALR_Production[production]->head;
		name=LALR_Symbol::id_to_LALR_Symbol[head]->name;
	}
	return name;
}
};
struct LALR_State
{
	static std::map<int,struct LALR_State* > id_to_LALR_State;

	int id;
	std::map<std::string,struct LALR_Action *> action_map;

	struct LALR_Action* action_of(std::string lookahead)
	{
		if(action_map.count(lookahead)!=0)
			return action_map[lookahead];
		else 
			return NULL;
	}
	static struct LALR_State* Start_State()
	{
		return id_to_LALR_State[1];
	}
	static struct LALR_State* new_()
	{
		struct LALR_State* s=new struct LALR_State;
		s->id=0;
		return s;
	}
};
struct Parser_State
{
	std::vector<struct LALR_State* > track;
	struct Token* lookahead;
	struct LALR_Action *action;

	char* base;
	char* end;
	char* cur;
	std::string file_path;
	int line;
	int error_num;
	void inc_line()
	{
		line++;
	}
	void reset()
	{
		track.clear();
		lookahead=NULL;
		action=NULL;

		base=end=cur=NULL;
		file_path.clear();
		line=0;
	}
	void setup_input(std::string file_name)
	{
		reset();
		int size;
		base=file_to_char_array(file_name,&size);
		end=base+size-1;
		cur=base;
		file_path=file_name;
		line=1;
		error_num=0;
	}
	std::string POSITION()
	{
		std::string text;
		text.append(file_path);
		text.append("(");
		text.append(int_to_string(line));
		text.append(")");
		return text;
	}
};
struct parese_scope
{
	struct parese_scope* upper;
	//<key,type(variable or typedef_name)>
	std::map<std::string,std::string> items;
void put(std::string name,std::string type)
{
	items[name]=type;
}
std::string get_(struct parese_scope* scope,std::string name)
{
	if(scope==NULL)
		return "undefined";
	if(scope->items.count(name)==0)
		return get_(scope->upper,name);
	return scope->items[name];
}
std::string get(std::string name)
{

	if(items.count(name)==0)
		return get_(upper,name);
	return items[name];
}
//static section
	static struct parese_scope *current;
	static struct parese_scope *global;
	static std::string typedef_name_trigger;
	static void enter_file_scope()
	{
		global=new struct parese_scope();
		global->upper=NULL;
		current=global;
	}
	static void enter_scope()
	{
		struct parese_scope *new_s;
		new_s=new struct parese_scope();
		new_s->upper=current;
		current=new_s;
	}
	static void exit_scope()
	{
		current=current->upper;
		assert(current!=NULL);
	}
};
class Syntax_Parser
{
public:
	static void TEST();
	Syntax_Parser();
	struct TRANSLATION_UNIT* syntax_parsing();
private:
	void setup_DFA_table();
	void setup_LALR_table();
	struct syntax_tree_builder* builder;
	void setup_builder(struct syntax_tree_builder* ts);

	struct Token *get_next_parse_token();
	struct Token *get_next_token();
	struct Token *get_next_token_();
	int parsing_call_back(int type);
	struct Parser_State PState;
};