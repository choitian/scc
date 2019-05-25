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

#include "..\..\lib\common.h"

#include "type.h"

struct IDENTIFIER
{
	int id;
	std::string kind;
	std::string linkage;
	std::string storage;
	std::string name;
	struct TYPE* type;
	int ref;
static struct iFUNCTION* function(struct TYPE* type,std::string name,std::string linkage,std::string status);
static struct iVARIABLE* variable(struct TYPE* type,std::string name,std::string linkage,std::string storage,
	struct INITIALIZER_DATA* initializer,std::string status);
static struct iVARIABLE* variable_enum_constant(struct TYPE* type,std::string name,int value);
static struct iVARIABLE* parameter(struct TYPE* type,std::string name);

static struct iMEMBER* member(struct TYPE* type,std::string name);

static struct iTYPEDEF_NAME* typedef_name(struct TYPE* type,std::string name);
	void inc_ref()
	{
		this->ref++;
	}
	IDENTIFIER()
	{
		static int _id=0;
		this->id=_id++;
		this->storage="static";
		this->ref=0;
	}
};
struct iVARIABLE:public IDENTIFIER
{
	struct INITIALIZER_DATA* initializer;

	std::string status;
	int stack_offset;
	bool is_enum_constant;
	int enum_constant_value;
	iVARIABLE(struct TYPE* type,std::string name,std::string linkage,std::string storage,
		struct INITIALIZER_DATA* initializer,std::string status)
	{
		this->kind="VARIABLE";
		this->name=name;
		this->type=type;
		this->initializer=initializer;
		this->linkage=linkage;
		this->storage=storage;
		this->status=status;
		this->stack_offset=0;
		this->is_enum_constant=false;
	}
};
struct iLABEL
{
	iLABEL(std::string name);
	bool defined;
	std::string name;
	struct IC_INS_BLOCK* iblock;
};
struct FUNCTION_SCOPE
{
	std::vector<struct stmt_SWITCH* > switch_stack;
	std::stack<struct IC_INS_BLOCK* > continue_points;
	std::stack<struct IC_INS_BLOCK* > break_points;
	std::vector<struct iLABEL *> label_list;
	std::vector<struct iVARIABLE*>	local_list;
	std::map<std::string,struct iLABEL *> label_map;
	struct iLABEL* get_label(std::string name)
	{
		struct iLABEL *the_label;
		if(label_map.find(name)==label_map.end())//if no exist,create.
		{
			the_label= new iLABEL(name);

			label_map.insert(std::pair<std::string,struct iLABEL *>(name,the_label));
			label_list.push_back(the_label);
		}else
			the_label=label_map[name];
		return the_label;
	}
};
struct iFUNCTION:public IDENTIFIER
{
	std::string linkage;
	std::string status;
	struct syntax_node* body;
	iFUNCTION(struct TYPE* type,std::string name,std::string linkage,std::string status)
	{
		this->kind="FUNCTION";
		this->name=name;
		this->linkage=linkage;
		this->type=type;
		this->status=status;
		this->body=NULL;

		this->current_block=NULL;
		this->end_block=NULL;
		this->discard_ins=false;

		this->current_asm_ins_buffer=NULL;
	}
	std::vector<struct IDENTIFIER*> *parameter_list();
	struct TYPE* return_type()
	{
		return static_cast<struct TYPE_FUNCTION*>(type)->type_;
	}
	//for checking function body;label,case break,contine statement;loop,switch breakable information.
	struct FUNCTION_SCOPE function_scope;
	void insert_local(struct iVARIABLE* local_variable);
	//all the function defined.
	static std::vector<struct iFUNCTION*> definition_list;
	static std::vector<struct iVARIABLE*> function_static_list;
	static std::vector<struct IDENTIFIER*> function_extern_list;
	//for current function check body
	static struct iFUNCTION* current_definition;

	bool discard_ins;
	struct IC_INS_BLOCK* current_block;
	std::vector<struct IC_INS_BLOCK*> ins_block_list;
	std::vector<struct IC_INS_BLOCK*> basic_block_list;
	void create_basic_block_list();
	void insert_block(struct IC_INS_BLOCK* block);
	void reset_current_block();
	struct IC_INS_BLOCK* end_block;
	void set_current_block(struct IC_INS_BLOCK* block);
	void insert_ins(struct IC_INS* ins);
	static void emit_asm_ins(std::string ins);
	std::string *current_asm_ins_buffer;
	std::string ins_buffer_head;
	std::string ins_buffer_body;
	std::string ins_buffer_end;

	void translate();
//debug;
	void output_IC(std::string *buffer);
	void show_basic_block_list();
};

struct iMEMBER:public IDENTIFIER
{
	size_t offset;
	bool is_bit_field;
	size_t bit_offset;
	size_t width;
	iMEMBER(struct TYPE* type,std::string name)
	{
		this->kind="MEMBER";
		this->name=name;
		this->type=type;

		this->offset=0;
		this->is_bit_field=false;
		this->bit_offset=0;
		this->width=1024;//illegal value.
	}
};
struct iTYPEDEF_NAME:public IDENTIFIER
{
	iTYPEDEF_NAME(struct TYPE* type,std::string name)
	{
		this->kind="TYPEDEF_NAME";
		this->name=name;
		this->type=type;
	}
};

struct INITIALIZER_ACTION
{
	bool is_base_deta;
	struct VAR* base_var;
	int deta;
	bool is_clear;
	bool is_copy_string;
	unsigned int size;
	unsigned int offset;
	struct TYPE* type;
	struct ADDRESS* value;
	std::string string_value;
	INITIALIZER_ACTION()
	{
		offset=0;
		type=NULL;
		value=NULL;
		is_clear=false;
		is_copy_string=false;
		is_base_deta=false;
	}
};
struct INITIALIZER_DATA
{
	bool is_static;
	struct iVARIABLE* base_variable;
	unsigned int last_ini_offset;//ini_offset +ini_size,mark the end
	unsigned int last_ini_bit_offset;//ini_bit_offset,mark the head
	struct TYPE* last_ini_bit_offset_type;//ini_bit_offset,mark the head
	std::vector<std::pair<struct iMEMBER*,struct INITIALIZER*>> bit_field_unit_ini_data;

	INITIALIZER_DATA()
	{
		last_ini_offset=0;
		last_ini_bit_offset=0;
		last_ini_bit_offset_type=NULL;
	}
	std::vector<struct INITIALIZER_ACTION*> action_list;
	void copy_action_base_deta(unsigned int offset,struct TYPE* type,struct VAR* base_var,int deta)
	{
		struct INITIALIZER_ACTION* action=new struct INITIALIZER_ACTION();
		action->offset=offset;
		action->type=type;
		action->is_base_deta=true;
		action->base_var=base_var;
		action->deta=deta;
		action_list.push_back(action);
	}
	void copy_action(unsigned int offset,struct TYPE* type,struct ADDRESS* value)
	{
		struct INITIALIZER_ACTION* action=new struct INITIALIZER_ACTION();
		action->offset=offset;
		action->type=type;
		action->value=value;
		action_list.push_back(action);
	}
	void copy_action_string(unsigned int offset,std::string value)
	{
		struct INITIALIZER_ACTION* action=new struct INITIALIZER_ACTION();
		action->offset=offset;
		action->string_value=value;
		action->is_copy_string=true;
		action_list.push_back(action);
	}
	void clear_action(unsigned int offset,unsigned int size)
	{
		struct INITIALIZER_ACTION* action=new struct INITIALIZER_ACTION();
		action->offset=offset;
		action->size=size;
		action->is_clear=true;
		action_list.push_back(action);
	}
};