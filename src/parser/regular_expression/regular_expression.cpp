#include "regular_expression.h"

namespace{
	union VALUE *new_value()
	{
		return new union VALUE;
	}
	struct NODE* new_node()
	{
		struct NODE* node=new struct NODE();
		return node;
	}
	struct NODE *make_node(std::string op,struct NODE *child_l,struct NODE *child_r)
	{
		struct NODE *node=new_node();
		node->atrribs["op"].txt=op;
		if(child_l!=NULL)
			node->atrribs["child_l"].node=child_l;
		if(child_r!=NULL)
			node->atrribs["child_r"].node=child_r;
		return node;
	}
	struct NODE *make_char_node(char ch)
	{
		struct NODE *node=new_node();
		node->atrribs["op"].txt="CHAR";
		node->atrribs["data"].i=ch;
		return node;
	}
	bool is_in(char ch,char set[],size_t size)
	{
		for(size_t i=0;i<size;i++)
		{
			if(set[i]==ch)
				return true;
		}
		return false;
	}
	bool bothIsDigit(char c1,char c2)
	{
		if(IsDigit(c1) && IsDigit(c2))
			return true;
		return false;
	}
	bool bothIsLetter(char c1,char c2)
	{
		if(IsLetter(c1) && IsLetter(c2))
			return true;
		return false;
	}
	struct NODE *augment_string(char from,char to)
	{
		struct NODE *expr=make_char_node(from);
		struct NODE *child_r;
		for(char ch=from+1;ch<=to;ch++)
		{
			child_r=make_char_node(ch);
			expr=make_node("OR",expr,child_r);
		}
		return expr;
	}
	char Not_character_FST[]={' ','\t','\r','\n','\v','\f',-1,
	'(',')','[',']','{','}','\"','.','^','$','*','+','?','|','/',};
	void add_if_not_exist(std::vector<struct NODE*> *nodes,struct NODE* node)
	{
		for(std::vector<struct NODE*>::iterator nodes_it=nodes->begin();nodes_it!=nodes->end();nodes_it++)
		{
			if(*nodes_it==node)
				return;
		}
		nodes->push_back(node);
	}
	void Union_vector(std::vector<struct NODE*> *to,std::vector<struct NODE*> *from)
	{
		for(std::vector<struct NODE*>::iterator nodes_it=from->begin();nodes_it!=from->end();nodes_it++)
		{
			add_if_not_exist(to,*nodes_it);
		}
	}
	void Union_vector_to_state(struct NODE* to_state,std::vector<struct NODE*> *from)
	{
		std::vector<struct NODE*> *to=&(to_state->atrribs["pos_vector"].nodes);
		for(std::vector<struct NODE*>::iterator nodes_it=from->begin();nodes_it!=from->end();nodes_it++)
		{
			if((*nodes_it)->atrribs["op"].txt=="ACTION")
			{
				if(to_state->atrribs["action"].i < (*nodes_it)->atrribs["action_priority"].i)
				{
					to_state->atrribs["action"].i=(*nodes_it)->atrribs["action_priority"].i;
					to_state->atrribs["action"].node=*nodes_it;
				}
			}
			add_if_not_exist(to,*nodes_it);
		}
	}
	bool is_in_state(struct NODE *state,struct NODE *pos)
	{
		std::vector<struct NODE*> *pos_vector=&(state->atrribs["pos_vector"].nodes);
		for(std::vector<struct NODE*>::iterator pos_vector_it=pos_vector->begin();pos_vector_it!=pos_vector->end();pos_vector_it++)
		{
			if(*pos_vector_it==pos)
				return true;
		}
		return false;
	}
	bool is_equal_state(struct NODE *state1,struct NODE *state2)
	{
		if(state1->atrribs["pos_vector"].nodes.size()!=state2->atrribs["pos_vector"].nodes.size())
			return false;
		std::vector<struct NODE*> *pos_vector=&(state1->atrribs["pos_vector"].nodes);
		for(std::vector<struct NODE*>::iterator pos_vector_it=pos_vector->begin();pos_vector_it!=pos_vector->end();pos_vector_it++)
		{
			if(!is_in_state(state2,*pos_vector_it))
				return false;
		}
		return true;
	}
	bool is_in_states(std::vector<struct NODE*> *states,struct NODE *state,struct NODE** the_state)
	{
		for(std::vector<struct NODE*>::iterator states_it=states->begin();states_it!=states->end();states_it++)
		{
			if(is_equal_state(*states_it,state))
			{
				*the_state=*states_it;
				return true;
			}
		}
		*the_state=state;
		return false;
	}
}//namespace
void regular_expression::expectFST(std::string kind)
{
	if(!is_in_FST(kind))
	{
		::DEBUG("expect current input in first_set of %s\n",kind.c_str());
	}
}
bool regular_expression::is_in_FST(std::string kind)
{

	if(kind=="expression")
	{
		if(is_in_FST("character"))
			return true;
		if(input_[0]=='\"')
			return true;
		if(input_[0]=='[')
			return true;
		if(input_[0]=='.')
			return true;
		if(input_[0]=='(')
			return true;
		if(input_[0]=='{')
			return true;
	}else if(kind=="character")
	{
		if(!is_in(input_[0],Not_character_FST,sizeof(Not_character_FST)/sizeof(Not_character_FST[0])))
			return true;
	}else if(kind=="declare")
	{
		if(IsLetter(input_[0]))
			return true;
	}else if(kind=="cat_expression")
	{
		return is_in_FST("expression");
	}else if(kind=="or_expression")
	{
		return is_in_FST("expression");
	}else
	{
		assert(0);
	}
	return false;
}
char regular_expression::escape_char()
{
	input_++;
	switch(input_[0])
	{
	case 'n':
		return '\n';
	case 't':
		return '\t';
	case 'v':
		return '\v';
	case 'b':
		return '\b';
	case 'r':
		return '\r';
	case 'f':
		return '\f';
	case 'a':
		return '\a';
	case '\\':
		return '\\';
	case '?':
		return '\?';
	case '\'':
		return '\'';
	case '\"':
		return '\"';
	default:
		return input_[0];
	}
}
void regular_expression::skip_white_space()
{
	int nskiped;
	int nlines;
	if(::skip_white_space(input_,this->cur_line_end_,1,1,1,&nskiped,&nlines) != -1)
	{
		input_ += nskiped;
		line_num_+=nlines;
	}
}
std::string regular_expression::get_id()
{
	std::string ID;
	if(IsLetter(input_[0]))
	{
		while( IsLetter(input_[0]) || IsDigit(input_[0]) )
		{
			ID.append(1,input_[0]);
			input_++;
		}
	}
	assert(!ID.empty());
	return ID;
}

void regular_expression::declaration()
{
	std::string ID=get_id();
	skip_white_space();
	declarations_[ID]=input_;
	while(input_[0]!='\n')
	{
		input_++;
	}
}
struct NODE *regular_expression::action()
{
	static int action_priority=65536;
	struct NODE *ac=::new_node();
	std::string ac_str;

	while(input_[0]!='"')
	{
		input_++;
	}
	input_++;
	ac_str=get_id();
	while(input_[0]!='}')
	{
		input_++;
	}
	input_++;
	ac->atrribs["op"].txt="ACTION";
	ac->atrribs["action_priority"].i=action_priority--;
	ac->atrribs["action"].txt=ac_str;
	return ac;
}
struct NODE* regular_expression::expression()
{
	struct NODE* expr;
	expr=or_expression();
	return expr;
}
struct NODE* regular_expression::or_expression()
{
	struct NODE* expr=cat_expression();
	struct NODE* child_r;
	if(input_[0]=='|')
	{
		input_++;
		expectFST("or_expression");
		child_r=or_expression();
		expr=::make_node("OR",expr,child_r);
	}
	return expr;
}
struct NODE* regular_expression::cat_expression()
{
	struct NODE* expr=postfix_expression();
	struct NODE* child_r;
	if(is_in_FST("cat_expression"))
	{
		child_r=cat_expression();
		expr=::make_node("CAT",expr,child_r);
	}
	return expr;
}
struct NODE* regular_expression::postfix_expression()
{
	struct NODE* expr=primary_expression();
	struct NODE* child_r;
	if(input_[0]=='*')
	{
		input_++;
		expr=::make_node("STAR",expr,NULL);
	}else if(input_[0]=='+')
	{
		input_++;
		child_r=::make_node("STAR",expr,NULL);
		expr=::make_node("CAT",expr,child_r);
	}else if(input_[0]=='?')
	{
		input_++;
		child_r=::make_node("NULL",NULL,NULL);
		expr=::make_node("OR",expr,child_r);
	}
	return expr;
}
struct NODE* regular_expression::primary_expression()
{
	struct NODE* expr;
	if(input_[0]=='"')
	{
		expr=string_literally();
	}else if(input_[0]=='[')
	{
		expr=string_array();
	}else if(input_[0]=='.')
	{
		expr=but_newline();
	}else if(input_[0]=='(')
	{
		input_++;
		expr=expression();
		input_++;
	}else if(is_in_FST("character"))
	{
		expr=character();
	}else if(input_[0]=='{')
	{
		input_++;
		std::string ID=get_id();
		if(declarations_.count(ID)==0)
		{
			::DEBUG("undefined declaration");
		}
		char *pos_saved=input_;
		input_=declarations_[ID];
		expr=expression();
		input_=pos_saved;
		input_++;
	}
	else
	{
		assert(0);
		return NULL;
	}
	return expr;
}
struct NODE* regular_expression::string_literally()
{
	struct NODE* expr = NULL;
	struct NODE* child_r;
	std::string str;
	char ch;

	input_++;
	if(input_[0]!='"')
	{
		if(input_[0]=='\\')
		{
			ch=escape_char();
		}else
		{
			ch=input_[0];
		}
		input_++;
		expr=make_char_node(ch);
	}
	while(input_[0]!='"')
	{
		if(input_[0]=='\\')
		{
			ch=escape_char();
		}else
		{
			ch=input_[0];
		}
		input_++;
		child_r=make_char_node(ch);
		expr=::make_node("CAT",expr,child_r);
	}
	input_++;
	return expr;
}
struct NODE* regular_expression::string_array()
{
	struct NODE* expr=NULL;
	struct NODE* child_r;
	std::string str;
	char ch;
	bool not_=false;

	input_++;
	if(input_[0]=='^')
	{
		not_=true;
		input_++;
	}
	while(input_[0]!=']')
	{
		if(input_[0]=='\\')
		{
			ch=escape_char();
		}else
		{
			ch=input_[0];
		}
		input_++;
		str.append(1,ch);
	}
	input_++;
	assert(!str.empty());
	size_t i;
	if(not_)
	{
		char mask[128];
		for(i=0;i<128;i++)
		{
			mask[i]=1;
		}
		for(i=0;i<str.size();i++)
		{
			char index=str[i];
			mask[index]=0;
		}
		str.clear();
		for(i=0;i<128;i++)
		{
			if(mask[i]==1)
			{
				str.append(1,i);
			}
		}
	}
	assert(!str.empty());
	i=0;
	if((i+2< str.size())&& str[i] < str[i+2] && str[i+1]=='-' && (bothIsDigit(str[i],str[i+2])||bothIsLetter(str[i],str[i+2])))
	{
		expr=augment_string(str[i],str[i+2]);
		i+=3;
	}else
	{
		expr=make_char_node(str[i]);
		i++;
	}
	while(i<str.size())
	{
		if((i+2< str.size()) && str[i] < str[i+2] && str[i+1]=='-' && (bothIsDigit(str[i],str[i+2])||bothIsLetter(str[i],str[i+2])))
		{
			child_r=augment_string(str[i],str[i+2]);
			i+=3;
		}else
		{
			child_r=make_char_node(str[i]);
			i++;
		}
		expr=make_node("OR",expr,child_r);
	}
	return expr;
}
struct NODE* regular_expression::character()
{
	struct NODE* expr=NULL;
	char ch;
	if(input_[0]=='\\')
	{
		ch=escape_char();
	}else
	{
		ch=input_[0];
	}
	input_++;
	expr=make_char_node(ch);
	return expr;
}
struct NODE* regular_expression::but_newline()
{
	struct NODE* expr=NULL;
	struct NODE* child_r;
	input_++;

	expr=make_char_node(0);
	size_t i;
	for(i=1;i<128;i++)
	{
		if(i!='\n')
		{ 
			child_r=make_char_node(i);
			expr=make_node("OR",expr,child_r);
		}
	}
	return expr;
}

char* regular_expression::new_line()
{
	int size;
	if(::one_line(this->input_cur_,this->input_end_,1,&size,NULL)!=-1)
	{
		cur_line_=this->input_cur_;
		cur_line_end_=cur_line_+size-1;
		input_=cur_line_;
		input_cur_ +=size;
		skip_white_space();
		if(input_ > cur_line_end_)
		{
			return new_line();
		}
		return cur_line_;
	}else
	{
		end_of_file=1;
	}
}
void regular_expression::do_build()
{
	while(1)
	{
		new_line();
		if(input_[0]=='%' &&input_[1]=='%')
		{
			break;
		}
		if(is_in_FST("declare"))
			declaration();
	}
	struct NODE *expr = NULL,*expr_r=NULL;
	struct NODE * ac;
	new_line();
	if(is_in_FST("expression"))
	{
		expr=expression();
		skip_white_space();
		ac=action();
		expr=make_node("CAT",expr,ac);
		new_line();
	}
	while(end_of_file==0)
	{
		expectFST("expression");
		expr_r=expression();
		skip_white_space();
		ac=action();
		expr_r=make_node("CAT",expr_r,ac);

		expr=make_node("OR",expr,expr_r);
		new_line();
	}
	RE_=expr;
}
void regular_expression::do_computing(struct NODE* node)
{
	static int num=1;
	if(node==NULL)
		return;
	if(node->atrribs.count("child_l"))
		do_computing(node->atrribs["child_l"].node);
	if(node->atrribs.count("child_r"))
		do_computing(node->atrribs["child_r"].node);

	std::string op=node->atrribs["op"].txt;
	if(op=="NULL")
	{
		node->atrribs["nullable"].i=1;
	}else if(op=="ACTION")
	{
		node->atrribs["firstpos"].nodes.push_back(node);
		node->atrribs["lastpos"].nodes.push_back(node);
		node->atrribs["num"].i=num++;
	}else if(op=="CHAR")
	{
		node->atrribs["firstpos"].nodes.push_back(node);
		node->atrribs["lastpos"].nodes.push_back(node);
		node->atrribs["num"].i=num++;
	}else if(op=="OR")
	{
		if(node->atrribs["child_l"].node->atrribs["nullable"].i==1 || node->atrribs["child_r"].node->atrribs["nullable"].i==1)
			node->atrribs["nullable"].i=1;
		else
			node->atrribs["nullable"].i=0;

		Union_vector(&(node->atrribs["firstpos"].nodes),&(node->atrribs["child_l"].node->atrribs["firstpos"].nodes));
		Union_vector(&(node->atrribs["firstpos"].nodes),&(node->atrribs["child_r"].node->atrribs["firstpos"].nodes));

		Union_vector(&(node->atrribs["lastpos"].nodes),&(node->atrribs["child_l"].node->atrribs["lastpos"].nodes));
		Union_vector(&(node->atrribs["lastpos"].nodes),&(node->atrribs["child_r"].node->atrribs["lastpos"].nodes));
		
	}else if(op=="CAT")
	{
		if(node->atrribs["child_l"].node->atrribs["nullable"].i==1 && node->atrribs["child_r"].node->atrribs["nullable"].i==1)
			node->atrribs["nullable"].i=1;
		else
			node->atrribs["nullable"].i=0;

		Union_vector(&(node->atrribs["firstpos"].nodes),&(node->atrribs["child_l"].node->atrribs["firstpos"].nodes));
		if(node->atrribs["child_l"].node->atrribs["nullable"].i==1)
			Union_vector(&(node->atrribs["firstpos"].nodes),&(node->atrribs["child_r"].node->atrribs["firstpos"].nodes));

		Union_vector(&(node->atrribs["lastpos"].nodes),&(node->atrribs["child_r"].node->atrribs["lastpos"].nodes));
		if(node->atrribs["child_r"].node->atrribs["nullable"].i==1)
			Union_vector(&(node->atrribs["lastpos"].nodes),&(node->atrribs["child_l"].node->atrribs["lastpos"].nodes));

		std::vector<struct NODE*> *nodes=&(node->atrribs["child_l"].node->atrribs["lastpos"].nodes);
		for(std::vector<struct NODE*>::iterator nodes_it=nodes->begin();nodes_it!=nodes->end();nodes_it++)
		{
			Union_vector(&((*nodes_it)->atrribs["followpos"].nodes),&(node->atrribs["child_r"].node->atrribs["firstpos"].nodes));
		}
	}else if(op=="STAR")
	{
		node->atrribs["nullable"].i=1;
		Union_vector(&(node->atrribs["firstpos"].nodes),&(node->atrribs["child_l"].node->atrribs["firstpos"].nodes));
		Union_vector(&(node->atrribs["lastpos"].nodes),&(node->atrribs["child_l"].node->atrribs["lastpos"].nodes));

		std::vector<struct NODE*> *nodes=&(node->atrribs["lastpos"].nodes);
		for(std::vector<struct NODE*>::iterator nodes_it=nodes->begin();nodes_it!=nodes->end();nodes_it++)
		{
			Union_vector(&((*nodes_it)->atrribs["followpos"].nodes),&(node->atrribs["firstpos"].nodes));
		}
	}
}
void catalog_followpos(std::map<char,struct NODE*> *catalogs,struct NODE* state)
{
	char ch;
	std::vector<struct NODE*> *nodes=&(state->atrribs["pos_vector"].nodes);
	std::vector<struct NODE*> *from;
	for(std::vector<struct NODE*>::iterator nodes_it=nodes->begin();nodes_it!=nodes->end();nodes_it++)
	{
		if((*nodes_it)->atrribs["op"].txt=="CHAR")
		{
			ch=(*nodes_it)->atrribs["data"].i;
			if(catalogs->count(ch)==0)
			{
				(*catalogs)[ch]=new_node();
			}
			from=&((*nodes_it)->atrribs["followpos"].nodes);
			Union_vector_to_state((*catalogs)[ch],from);
		}
	}
}
void regular_expression::add_state(struct NODE* state)
{
	static int id=1;
	state->atrribs["id"].i=id++;
	Dstates.push_back(state);
}
void regular_expression::construction_DFA()
{
	std::stack<struct NODE*> unmarked;
	std::map<char,struct NODE*> catalogs;
	struct NODE* state=::new_node();
	struct NODE* the_state;
	Union_vector_to_state(state,&(RE_->atrribs["firstpos"].nodes));
	add_state(state);
	unmarked.push(state);
	while(unmarked.size()!=0)
	{
		state=unmarked.top();
		unmarked.pop();
		catalogs.clear();
		catalog_followpos(&catalogs,state);
		for(std::map<char,struct NODE*>::iterator catalogs_it=catalogs.begin();catalogs_it!=catalogs.end();catalogs_it++)
		{
			std::string index;
			if(!is_in_states(&Dstates,catalogs_it->second,&the_state))
			{
				add_state(catalogs_it->second);
				unmarked.push(catalogs_it->second);
			}
			index.assign(1,catalogs_it->first);
			state->atrribs[index].node=the_state;
		}
	}
}
void regular_expression::out_DFA_table(std::string to_file)
{
	for(std::vector<struct NODE*>::iterator Dstates_it=Dstates.begin();Dstates_it!=Dstates.end();Dstates_it++)
	{
		out_DFA_state(*Dstates_it);
	}
	string_to_file(DFA_table_,to_file);
}
void regular_expression::out_DFA_state(struct NODE* state)
{
	DFA_table_.append(int_to_string(state->atrribs["id"].i));
	DFA_table_.append(":");
	if(state->atrribs["action"].i!=0)
	{
		DFA_table_.append(state->atrribs["action"].node->atrribs["action"].txt);
	}
	DFA_table_.append(":");
	for(std::map<std::string,struct OBJ>::iterator atrribs_it=state->atrribs.begin();atrribs_it!=state->atrribs.end();atrribs_it++)
	{
		if(atrribs_it->first.size()!=1)
		{
			continue;
		}
		DFA_table_.append("[");
		DFA_table_.append(atrribs_it->first);
		DFA_table_.append(",");
		DFA_table_.append(int_to_string(atrribs_it->second.node->atrribs["id"].i));
		DFA_table_.append("]");
	}
	DFA_table_.append(";");
	DFA_table_.append("\n");
}

regular_expression::regular_expression(std::string RE_file,std::string to_DFA_state_file)
{
	line_num_=0;
	end_of_file=0;
	int size;
	input_base_=file_to_char_array(RE_file,&size);
	input_cur_=input_base_;
	input_end_=input_base_+size-1;


	DEBUG("do build...\n");
	do_build();
	DEBUG("done!\n\n\n");

	DEBUG("do computing...\n");
	do_computing(RE_);
	DEBUG("done!\n\n\n");

	DEBUG("construction DFA...\n");
	construction_DFA();
	DEBUG("done!\n\n\n");

	DEBUG("out DFA_table to: %s...\n",to_DFA_state_file.c_str());
	out_DFA_table(to_DFA_state_file);
	DEBUG("done!\n\n\n");
}
void regular_expression::TEST()
{
	regular_expression re("C_RE.txt","DFA_state.txt");
	system("PAUSE");
}