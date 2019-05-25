#pragma once
#include "..\\lib\\common.h"
union VALUE
{
	char c;
	unsigned char uc;
	short s;
	unsigned short us;
	long l;
	unsigned long ul;
	float f;
	double d;
};
struct NODE;
struct OBJ
{
	int i;
	double d;
	std::string txt;
	struct NODE* node;
	std::vector<struct NODE*> nodes;
	void* extra;
	OBJ()
	{
		i=0;
		d=0.0;
		node=NULL;
		extra=NULL;
	}
};
struct NODE
{
	std::map<std::string,struct OBJ> atrribs;
static struct NODE* new_node()
{
	return new struct NODE();
}
//int_R:int's reference
//int_:int's value
//npv_R:node's pointer's vector's reference
	int* int_R(std::string field_name)
	{
		return &(this->atrribs[field_name].i);
	}
	int int_(std::string field_name)
	{
		return this->atrribs[field_name].i;
	}
	double* dob_R(std::string field_name)
	{
		return &(this->atrribs[field_name].d);
	}
	double dob(std::string field_name)
	{
		return this->atrribs[field_name].d;
	}
	std::string* txt_R(std::string field_name)
	{
		return &(this->atrribs[field_name].txt);
	}
	std::string txt(std::string field_name)
	{
		return this->atrribs[field_name].txt;
	}
	struct NODE** np_R(std::string field_name)
	{
		return &(this->atrribs[field_name].node);
	}
	struct NODE* np(std::string field_name)
	{
		return this->atrribs[field_name].node;
	}
	void** vp_R(std::string field_name)
	{
		return &(this->atrribs[field_name].extra);
	}
	void* vp(std::string field_name)
	{
		return this->atrribs[field_name].extra;
	}
	std::vector<struct NODE*>* npv_R(std::string field_name)
	{
		return &(this->atrribs[field_name].nodes);
	}
};
class regular_expression
{
public:
	static void TEST();
	regular_expression(std::string RE_file,std::string to_DFA_state_file);
	std::vector<struct NODE*> Dstates;
private:
//tets
//driver
	void do_build();
	void do_computing(struct NODE* node);
	void construction_DFA();
	void out_DFA_table(std::string to_file);
//function
	char escape_char();
	void expectFST(std::string kind);
	bool is_in_FST(std::string kind);
	void skip_white_space();
	void skip_white_space_src();
	std::string get_id();

	void declaration();
	struct NODE * action();
	struct NODE* expression();
	struct NODE* or_expression();
	struct NODE* cat_expression();
	struct NODE* postfix_expression();
	struct NODE* primary_expression();
	struct NODE* string_literally();
	struct NODE* string_array();
	struct NODE* character();
	struct NODE* but_newline();

	void add_state(struct NODE* state);
	void out_DFA_state(struct NODE* state);

	char* new_line();
//data
	std::map<std::string,char*> declarations_;
	int line_num_;
	char* input_cur_;
	char* input_base_;
	char* input_end_;
	char* cur_line_,*cur_line_end_;
	char* input_;

	struct NODE* RE_;

	int end_of_file;
	std::string DFA_table_;
};