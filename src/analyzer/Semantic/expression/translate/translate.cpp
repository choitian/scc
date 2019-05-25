#include <Cstring>
#include <Cstdlib> 
#include <Cstdio> 
#include <Cassert>
#include <Cstdarg>
#include <Climits>
#include <Cctype>

#include <string> 
#include <vector> 
#include <set> 
#include <list>
#include <queue>
#include <stack> 
#include <map>
#include <algorithm>

#include "..\IC.h"
#include "register.h"
#include "translate.h"

static std::vector<struct IDENTIFIER*> floating_constant_list;
struct MEM* new_floating(struct CONST* value)
{
	struct INITIALIZER_DATA* initializer=new struct INITIALIZER_DATA();
	initializer->copy_action(0,TYPE::basic_type("float64"),value);
	struct MEM* mem=MEM::global_temp(TYPE::basic_type("float64"),initializer);
	return mem;
}
std::string address_ptr(unsigned int width)
{
	if(width==1)
		return "BYTE PTR";
	else if(width==2)
		return "WORD PTR";
	else if(width==4)
		return "DWORD PTR";
	else if(width==8)
		return "QWORD PTR";
	assert(0);
	return "";
}
void IC_INS::pre_translate()
{
}
void IC_INS::post_translate()
{
	this->dec_ref();
}
void translate_ins(struct IC_INS* ins)
{
	ins->pre_translate();
	iFUNCTION::emit_asm_ins(string_format(";%s",ins->toString().c_str()));
	ins->translate();
	ins->post_translate();
}
void IC_INS_BLOCK::translate()
{
	if(this->ref>0)
		iFUNCTION::emit_asm_ins(string_format("%s:",this->asm_name().c_str()));
	
	for(size_t i=0;i<ins_list.size()-1;i++)
	{
		translate_ins(ins_list.at(i));
	}
	
	struct IC_INS* ins=ins_list.back();
	//special for last ins.
	//must put the code that save data BEFORE jump ins code.
	//so let the ins->translate call spill_all
	if(ins->is_jump_ins())
	{
		ins->pre_translate();
		iFUNCTION::emit_asm_ins(string_format(";%s",ins->toString().c_str()));
		ins->translate();
	}else
	{
		translate_ins(ins);
		x86_reg.spill_all();
	}
}

static void emit_local_offset_name(struct iVARIABLE* v)
{
	iFUNCTION::emit_asm_ins(string_format("%s = %d \t;size: %u",
		get_access_name(v).c_str(),v->stack_offset,v->type->size));
}
void calculate_stack_offset_parameter(struct iFUNCTION* function)
{
	size_t offset=FIRST_PARAMETER_OFFSET;
	std::vector<struct IDENTIFIER*> *parament_list=function->parameter_list();
	struct iVARIABLE* parameter;
	for(std::vector<struct IDENTIFIER*>::iterator it=parament_list->begin();it!=parament_list->end();it++)
	{
		parameter=static_cast<struct iVARIABLE*>(*it);
		offset=ALIGN(offset,PARAMETER_ALIGN);
		parameter->stack_offset=offset;
		offset+=parameter->type->size;
		emit_local_offset_name(parameter);
	}
}
int calculate_stack_offset_local(struct iFUNCTION* function)
{
	std::vector<struct iVARIABLE*> *local_list=&(function->function_scope.local_list);
	size_t offset=0;
	for(std::vector<struct iVARIABLE*>::iterator it=local_list->begin();it!=local_list->end();it++)
	{
		struct iVARIABLE* v=*it;
		if(v->storage=="static")
		{

		}else
		{
			offset=ALIGN(offset,v->type->align);
			offset +=v->type->size;
			v->stack_offset=-((int)offset);
			emit_local_offset_name(v);
		}
	}
	offset=ALIGN(offset,STACK_ALIGN_SIZE);
	return offset;
}
unsigned int emit_proc_head(struct iFUNCTION* function)
{
	iFUNCTION::emit_asm_ins(string_format("_TEXT	SEGMENT"));
	calculate_stack_offset_parameter(function);
	unsigned int reserve_size=calculate_stack_offset_local(function);
	iFUNCTION::emit_asm_ins(string_format("_%s   PROC",function->name.c_str()));
	return reserve_size;
}
void emit_proc_tail(struct iFUNCTION* function)
{
	iFUNCTION::emit_asm_ins(string_format("_%s   ENDP",function->name.c_str()));
	iFUNCTION::emit_asm_ins(string_format("_TEXT	ENDS"));
}

void emit_prologue(unsigned int reserve_size)
{
	iFUNCTION::emit_asm_ins(string_format("push ebp"));
	iFUNCTION::emit_asm_ins(string_format("push ebx"));
	iFUNCTION::emit_asm_ins(string_format("push esi"));
	iFUNCTION::emit_asm_ins(string_format("push edi"));
	iFUNCTION::emit_asm_ins(string_format("mov ebp,esp"));
	iFUNCTION::emit_asm_ins(string_format("sub esp,%d",reserve_size));
}
void emit_epilogue()
{
	iFUNCTION::emit_asm_ins(string_format("mov esp,ebp"));
	iFUNCTION::emit_asm_ins(string_format("pop edi"));
	iFUNCTION::emit_asm_ins(string_format("pop esi"));
	iFUNCTION::emit_asm_ins(string_format("pop ebx"));
	iFUNCTION::emit_asm_ins(string_format("pop ebp"));
	iFUNCTION::emit_asm_ins(string_format("ret 0"));
}
void iFUNCTION::translate()
{
/*
	proc_head
	Prologue

	body

	Epilogue
	proc_tail
*/
	this->current_asm_ins_buffer=&this->ins_buffer_body;
	for(size_t i=0;i<basic_block_list.size();i++)
	{
		basic_block_list.at(i)->translate();
	}

	this->current_asm_ins_buffer=&this->ins_buffer_head;
	int  reserve_size=emit_proc_head(this);
	emit_prologue(reserve_size);

	this->current_asm_ins_buffer=&this->ins_buffer_end;
	emit_epilogue();
	emit_proc_tail(this);
}

static std::string *current_program_buffer=NULL;
static std::string program_head;
static std::string program_body;
static std::string ic_buffer;
void program_append(std::string txt)
{
	assert(current_program_buffer!=NULL);
	current_program_buffer->append(txt);
}
void program_print(std::string txt)
{
	assert(current_program_buffer!=NULL);
	current_program_buffer->append(string_format("%s\n",txt.c_str()));
}
void begin_procgram()
{
	program_print(string_format(";x86 asm code."));
	program_print(string_format(".686P"));
	program_print(string_format(".XMM"));
	program_print(string_format(".model flat"));
	program_print(string_format("EXTRN __fltused:NEAR32"));
	program_print(string_format("EXTRN _memset:NEAR32"));
}
void end_procgram()
{
	program_print(string_format("END"));
}
void import_symbol(struct IDENTIFIER *symbol)
{
	if(symbol->ref >0)
	{
		std::string access_name=get_access_name(symbol);
		program_print(string_format("EXTERN\t%s:NEAR32",access_name.c_str()));
	}
}
void export_symbol(std::string access_name)
{
	program_print(string_format("PUBLIC\t%s",access_name.c_str()));
}
void define_comm_data(struct iVARIABLE *v)
{
	
	std::string access_name=get_access_name(v);
	 if(v->linkage=="external")
	{
		program_print(string_format("COMM\t%s:%u",access_name.c_str(),v->type->size));
	}else
	{
		program_print(string_format("%s\tDB %u DUP (0)",access_name.c_str(),v->type->size));
	}
}
std::string TYPE_ABBREVIATION(unsigned int size)
{
	std::string txt;
	switch(size)
	{
	case 1:
		txt="DB";
		break;
	case 2:
		txt="DW";
		break;
	case 4:
		txt="DD";
		break;
	case 8:
		txt="DQ";
		break;
	default:
		txt="DB";
		break;
	};
	return txt;
}
static std::string value_text(struct ADDRESS* addr)
{

	assert(addr->is_CONST());
	struct CONST* con=static_cast<struct CONST*>(addr);
	if(con->type->is_floating_type())
	{
		if(con->type->size==4)
			return string_format("0%08xr\t;%f",con->value.xx[0],con->value.float32);
		else
			return string_format("0%08x%08xr\t;%f",con->value.xx[1],con->value.xx[0],con->value.float64);
	}else
	{
		return string_format("0%xH",con->as_uint32());
	}
}
static char escape_char(char escape_char)
{
	char value;
	switch(escape_char)
	{
		case 'n':
			value='\n';break;
		case 't':
			value='\t';break;
		case 'v':
			value='\v';break;
		case 'b':
			value='\b';break;
		case 'r':
			value='\r';break;
		case 'f':
			value='\f';break;
		case 'a':
			value='\a';break;
		case '0':
			value='\0';break;
		case '\\':
		case '?':
		case '\'':
		case '\"':
			value=escape_char;break;
		default:
			value=escape_char;
	}
	return value;
}
bool next_char(std::string::iterator &cur,std::string::iterator end,char &c)
{
	if(cur==end)
	{
		return false;
	}
	if(*cur=='\\')
	{
		cur++;
		c=escape_char(*cur);
	}
	else
		c=*cur;
	cur++;
	return true;
}
void emit_string(std::string value)
{
	program_append("\tDB\t");
	char c;
	std::string::iterator it=value.begin();
	std::string::iterator end=value.end();
	while(next_char(it,end,c))
	{
		if(isprint(c))
		{
			program_append("'");
			while(isprint(c))
			{
				if(c=='\'')
					program_append("''");
				else
					program_append(string_format("%c",c));
				if(!next_char(it,end,c))
					break;
			}
			program_append("',");		
		}
		program_append(string_format("0%xH",c));
		if(it<end)
			program_append(",");
	}
	program_append(string_format("\t;%s\n",value.c_str()));
}
void emit_ini_value(struct INITIALIZER_ACTION* action)
{
	if(action->is_clear)
	{
		program_print(string_format("\tDB %u DUP (0)",action->size));
	}else
	{
		if(action->is_copy_string)
		{
			emit_string(action->string_value);
		}else
		{
			std::string abbr=TYPE_ABBREVIATION(action->type->size);
			if(action->is_base_deta)
			{
				if(action->deta==0)
				{
					program_print(string_format("\t%s\t%s",abbr.c_str(),get_access_name(action->base_var->var).c_str()));
				}else
				{
					program_print(string_format("\t%s\t%s%+d",abbr.c_str(),get_access_name(action->base_var->var).c_str(),action->deta));
				}
			}else
				program_print(string_format("\t%s\t%s",abbr.c_str(),value_text(action->value).c_str()));
		}
	}
}
void define_init_data(struct iVARIABLE *v)
{
	std::string access_name=get_access_name(v);
	if(v->linkage=="external")
	{
		export_symbol(access_name);
	}
	//action list 
	program_append(access_name);
	for(std::vector<struct INITIALIZER_ACTION*>::iterator it=v->initializer->action_list.begin();
		it!=v->initializer->action_list.end();it++)
	{
		emit_ini_value(*it);
	}
}
void define_global(struct iVARIABLE *v)
{
	if(v->status=="declaration")
	{
		import_symbol(v);
	}else 
	{
		if(v->initializer==NULL)
		{
			define_comm_data(v);
		}else
		{
			define_init_data(v);
		}
	}
}

void Semantic_Analyzer::translate()
{
/*
head
strings
floatings
globals
*/
	current_program_buffer=&program_body;
	for(std::vector<struct iFUNCTION*>::iterator it=iFUNCTION::definition_list.begin();
		it!=iFUNCTION::definition_list.end();it++)
	{
		struct iFUNCTION* fun=*it;
		iFUNCTION::current_definition=fun;
		if(fun->linkage=="external")
		{
			export_symbol(::get_access_name(fun));
		}
		fun->output_IC(&ic_buffer);
		fun->translate();
		program_append(fun->ins_buffer_head);
		program_append(fun->ins_buffer_body);
		program_append(fun->ins_buffer_end);
	}
	end_procgram();

	current_program_buffer=&program_head;
	begin_procgram();
	program_print("_DATA  SEGMENT");	 
	for(std::vector<struct IDENTIFIER*>::iterator it=iFUNCTION::function_extern_list.begin();it!=iFUNCTION::function_extern_list.end();it++)
	{
		struct IDENTIFIER* old=SCOPE::file->find((*it)->name);
		if(old ==NULL || old->kind=="TYPEDEF_NAME")
		{
			SCOPE::file->insert((*it)->name,*it);
		}
	}
	std::vector<struct IDENTIFIER*> *global_symbols=SCOPE::file->symbol_list();
	for(std::vector<struct IDENTIFIER*>::iterator it=global_symbols->begin();it!=global_symbols->end();it++)
	{
		struct IDENTIFIER* symbol=*it;
		if(symbol->kind=="VARIABLE")
		{
			struct iVARIABLE *v=static_cast<struct iVARIABLE *>(symbol);
			define_global(v);
		}
		else if(symbol->kind=="FUNCTION")
		{
			struct iFUNCTION *fun=static_cast<struct iFUNCTION *>(symbol);
			if(fun->body==NULL )
			{
				import_symbol(fun);
			}
		}
	}
	for(std::vector<struct iVARIABLE*>::iterator it=iFUNCTION::function_static_list.begin();it!=iFUNCTION::function_static_list.end();it++)
	{
		define_global(*it);
	}
	program_print("_DATA  ENDS");
	std::string program;
	program.append(program_head);
	program.append(program_body);
	string_to_file(ic_buffer,SCC_ENV.ic_file);
	string_to_file(program,SCC_ENV.asm_file);
}



