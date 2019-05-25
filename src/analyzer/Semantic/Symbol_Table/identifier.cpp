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

#include "identifier.h"
#include "..\expression\IC.h"

std::vector<struct iFUNCTION*> iFUNCTION::definition_list;
std::vector<struct iVARIABLE*> iFUNCTION::function_static_list;
std::vector<struct IDENTIFIER*> iFUNCTION::function_extern_list;
struct iFUNCTION global_dummy_function(NULL,"global_dummy_function","external","declaration");
struct iFUNCTION* iFUNCTION::current_definition=&global_dummy_function;

struct iFUNCTION* IDENTIFIER::function(struct TYPE* type,std::string name,std::string linkage,std::string status)
{
	return new struct iFUNCTION(type,name,linkage,status);
}
struct iVARIABLE* IDENTIFIER::variable(struct TYPE* type,std::string name,std::string linkage,std::string storage,
	struct INITIALIZER_DATA* initializer,std::string status)
{
	return new iVARIABLE(type,name,linkage,storage,initializer,status);
}
struct iVARIABLE* IDENTIFIER::variable_enum_constant(struct TYPE* type,std::string name,int value)
{
	struct iVARIABLE* enum_constant=new iVARIABLE(type,name,"none","static",NULL,"none");
	enum_constant->is_enum_constant=true;
	enum_constant->enum_constant_value=value;
	return enum_constant;
}
struct iVARIABLE* IDENTIFIER::parameter(struct TYPE* type,std::string name)
{
	return IDENTIFIER::variable(type,name,"none","automatic",NULL,"none");
}
struct iMEMBER* IDENTIFIER::member(struct TYPE* type,std::string name)
{
	return new iMEMBER(type,name);
}

struct iTYPEDEF_NAME* IDENTIFIER::typedef_name(struct TYPE* type,std::string name)
{
	return new iTYPEDEF_NAME(type,name);
}
void iFUNCTION::insert_local(struct iVARIABLE* local_variable)
{
	this->function_scope.local_list.push_back(local_variable);
}

void iFUNCTION::show_basic_block_list()
{
	::SCC_MSG("\n(FUN)%s:",this->name.c_str());
	for(size_t i=0;i<basic_block_list.size();i++)
	{
		basic_block_list.at(i)->show();
	}
}
void iFUNCTION::output_IC(std::string *buffer)
{
	buffer->append(string_format("\n(FUN)%s\n:",this->name.c_str()));
	for(size_t i=0;i<basic_block_list.size();i++)
	{
		basic_block_list.at(i)->output_IC(buffer);
	}
}
std::vector<struct IDENTIFIER*> *iFUNCTION::parameter_list()
{
	struct TYPE_FUNCTION* function_type=static_cast<struct TYPE_FUNCTION*>(type);
	return &(function_type->prototype_->list_);
}
void iFUNCTION::create_basic_block_list()
{
	static int id=0;
	struct IC_INS_BLOCK* new_basic_block=new IC_INS_BLOCK();
	new_basic_block->name=string_format("L_%d",id++);
	this->basic_block_list.push_back(new_basic_block);
	std::queue<struct IC_INS_BLOCK*> refed_block_queue;
	bool see_jump=false;
	for(size_t block_i=0;block_i<ins_block_list.size();block_i++)
	{
		struct IC_INS_BLOCK* ins_block=ins_block_list.at(block_i);
		if(ins_block->ref>0)
		{
			refed_block_queue.push(ins_block);
		}
		for(size_t ins_i=0;ins_i<ins_block->ins_list.size();ins_i++)
		{
			struct IC_INS* ins=ins_block->ins_list.at(ins_i);
			if((!refed_block_queue.empty()) || see_jump)
			{
				new_basic_block=new IC_INS_BLOCK();
				new_basic_block->name=string_format("L_%d",id++);
				this->basic_block_list.push_back(new_basic_block);
				
				while(!refed_block_queue.empty())
				{
					struct IC_INS_BLOCK* rerefed_block=refed_block_queue.front();
					refed_block_queue.pop();
					for(size_t ref_i=0;ref_i<rerefed_block->reference_list.size();ref_i++)
					{
						*(rerefed_block->reference_list.at(ref_i))=new_basic_block;
						new_basic_block->reference(rerefed_block->reference_list.at(ref_i));
					}
				}
				see_jump=false;
			}
			if(ins->is_jump_ins())
			{
				see_jump=true;
			}
			new_basic_block->insert(ins);
		}
	}
}
void iFUNCTION::insert_block(struct IC_INS_BLOCK* block)
{
	if(!discard_ins)
	{
		ins_block_list.push_back(block);
		set_current_block(block);
	}
}
void iFUNCTION::reset_current_block()
{
	if(!ins_block_list.empty())
		current_block=ins_block_list.back();
}
void iFUNCTION::set_current_block(struct IC_INS_BLOCK* block)
{
	current_block=block;
}
void iFUNCTION::insert_ins(struct IC_INS* ins)
{
	if(!discard_ins && current_block!=NULL)
		current_block->insert(ins);
}
iLABEL::iLABEL(std::string name)
{
	this->name=name;
	this->iblock=IC_INS_BLOCK::new_(name);
	this->defined=false;
}
void iFUNCTION::emit_asm_ins(std::string ins)
{
	std::string *buffer=iFUNCTION::current_definition->current_asm_ins_buffer;
	assert(buffer!=NULL);
	buffer->append(string_format("%s\n",ins.c_str()));
}