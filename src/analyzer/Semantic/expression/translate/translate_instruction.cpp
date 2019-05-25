#include <Cstring>
#include <Cstdlib> 
#include <Cstdio> 
#include <Cassert>
#include <Cstdarg>
#include <Climits>

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

static void load_to_reg(struct ADDRESS* op,struct REG* reg)
{
	unsigned int width=op->type->size;
	if(op->is_TEMP())
	{
		struct TEMP* temp=static_cast<struct TEMP*>(op);
		std::string asm_op;
		if(temp->reg!=NULL)
		{
			if(temp->reg->d_name!=reg->d_name)
			{
				iFUNCTION::emit_asm_ins(string_format("mov %s,%s",reg->as(width).c_str(),temp->reg->as(width).c_str()));		
			}
		}else
		{
			load_to_reg(temp->mem,reg); 
		}
	}else
	{
		if(op->is_CONST())
		{
			struct CONST* con=static_cast<struct CONST* >(op);
			std::string asm_op=con->asm_value();
			iFUNCTION::emit_asm_ins(string_format("mov %s,%s",reg->as(width).c_str(),
				asm_op.c_str()));
		}else if(op->is_VAR())
		{
			struct VAR* var=static_cast<struct VAR*>(op);
			var->load_to_reg(reg);
		}else if(op->is_MEM())
		{
			struct MEM* mem=static_cast<struct MEM*>(op);
			std::string asm_op=mem->asm_value(reg);
			iFUNCTION::emit_asm_ins(string_format("mov %s,%s %s",reg->as(width).c_str(),
				address_ptr(width).c_str(),asm_op.c_str()));
		}else if(op->is_BIT_FIELD())
		{
			struct BIT_FIELD* bf=static_cast<struct BIT_FIELD*>(op);
			bf->load_to_reg(reg);
		}
	}
}
static struct REG* load_to_reg(struct ADDRESS* op)
{
	struct REG* reg;
	unsigned int width=op->type->size;
	if(op->is_TEMP())
	{
		struct TEMP* temp=static_cast<struct TEMP*>(op);
		std::string asm_op;
		if(temp->reg!=NULL)
		{
			reg=temp->reg;
		}else
		{
			reg=x86_reg.get_reg();
			std::string asm_op=temp->mem->asm_value();
			iFUNCTION::emit_asm_ins(string_format("mov %s,%s %s",reg->as(width).c_str(),
				address_ptr(width).c_str(),asm_op.c_str()));
		}
	}else
	{
		reg=x86_reg.get_reg();
		if(op->is_CONST())
		{
			struct CONST* con=static_cast<struct CONST* >(op);
			std::string asm_op=con->asm_value();
			iFUNCTION::emit_asm_ins(string_format("mov %s,%s",reg->as(4).c_str(),
				asm_op.c_str()));
		}else if(op->is_VAR())
		{
			struct VAR* var=static_cast<struct VAR*>(op);
			var->load_to_reg(reg);
		}else if(op->is_MEM())
		{
			struct MEM* mem=static_cast<struct MEM*>(op);
			std::string asm_op=mem->asm_value(reg);
			iFUNCTION::emit_asm_ins(string_format("mov %s,%s %s",reg->as(width).c_str(),
				address_ptr(width).c_str(),asm_op.c_str()));
		}else if(op->is_BIT_FIELD())
		{
			struct BIT_FIELD* bf=static_cast<struct BIT_FIELD*>(op);
			bf->load_to_reg(reg);
		}
	}
	return reg;
}
static struct REG* upcast(unsigned int result_width,struct ADDRESS* op)
{
	struct REG* result_reg;
	std::string asm_code;
	if(op->type->is_unsigned())
		asm_code="movzx";
	else
		asm_code="movsx";
	unsigned int source_width=op->type->size;
	if(op->is_VAR())
	{
		result_reg=load_to_reg(op);
		iFUNCTION::emit_asm_ins(string_format("%s %s,%s",
			asm_code.c_str(),
			result_reg->as(result_width).c_str(),
			result_reg->as(source_width).c_str()));
	}else if(op->is_MEM())
	{
		result_reg=x86_reg.get_reg();
		struct MEM* mem=static_cast<struct MEM*>(op);
		std::string asm_op=mem->asm_value(result_reg);
		iFUNCTION::emit_asm_ins(string_format("%s %s,%s %s",
			asm_code.c_str(),
			result_reg->as(result_width).c_str(),
			address_ptr(source_width).c_str(),
			asm_op.c_str()));
	}else if(op->is_BIT_FIELD())
	{
		result_reg=load_to_reg(op);
		iFUNCTION::emit_asm_ins(string_format("%s %s,%s",
			asm_code.c_str(),
			result_reg->as(result_width).c_str(),
			result_reg->as(source_width).c_str()));
	}else if(op->is_TEMP())
	{
		struct TEMP* temp=static_cast<struct TEMP*>(op);
		std::string asm_op;
		if(temp->reg!=NULL)
		{
			result_reg=temp->reg;
			iFUNCTION::emit_asm_ins(string_format("%s %s,%s",
				asm_code.c_str(),
				result_reg->as(result_width).c_str(),
				result_reg->as(source_width).c_str()));		
		}else
		{
			result_reg=x86_reg.get_reg();
			asm_op=temp->mem->asm_value();
			iFUNCTION::emit_asm_ins(string_format("%s %s,%s %s",
				asm_code.c_str(),
				result_reg->as(result_width).c_str(),
				address_ptr(source_width).c_str(),
				asm_op.c_str()));
		}
	}
	return result_reg;
}
static struct REG* downcast(unsigned int result_width,struct ADDRESS* op)
{
	struct REG* result_reg;
	if(op->is_CONST())
	{
		result_reg=load_to_reg(op);
	}else if(op->is_VAR())
	{
		result_reg=load_to_reg(op);
	}else if(op->is_MEM())
	{
		struct MEM* mem=static_cast<struct MEM*>(op);
		result_reg=x86_reg.get_reg();
		std::string asm_op=mem->asm_value(result_reg);
		iFUNCTION::emit_asm_ins(string_format("mov %s,%s %s",
			result_reg->as(result_width).c_str(),
			address_ptr(result_width).c_str(),
			asm_op.c_str()));
	}else if(op->is_BIT_FIELD())
	{
		result_reg=load_to_reg(op);
	}
	else if(op->is_TEMP())
	{
		struct TEMP* temp=static_cast<struct TEMP*>(op);
		std::string asm_op;
		if(temp->reg!=NULL)
		{
			result_reg=temp->reg;
		}else
		{
			result_reg=x86_reg.get_reg();
			asm_op=temp->mem->asm_value();
			iFUNCTION::emit_asm_ins(string_format("mov %s,%s %s",
				result_reg->as(result_width).c_str(),
				address_ptr(result_width).c_str(),
				asm_op.c_str()));
		}
	}
	return result_reg;
}
static struct REG* asm_standard_binary(std::string asm_code,struct ADDRESS* op0,struct ADDRESS* op1)
{
//load op0 to result_reg
	struct REG* result_reg=load_to_reg(op0);
//applay asm_code on result_reg,op1
	result_reg->lock();
	std::string asm_op1;
	if(op1->is_CONST())
	{
		struct CONST* con=static_cast<struct CONST*>(op1);
		asm_op1=con->asm_value();
		iFUNCTION::emit_asm_ins(string_format("%s %s,%s",asm_code.c_str(),result_reg->as(4).c_str(),asm_op1.c_str()));
	}else if(op1->is_VAR())
	{
		struct REG* reg_new=load_to_reg(op1);
		iFUNCTION::emit_asm_ins(string_format("%s %s,%s",asm_code.c_str(),result_reg->as(4).c_str(),reg_new->as(4).c_str()));
	}else if(op1->is_MEM())
	{
		struct MEM* mem=static_cast<struct MEM*>(op1);
		asm_op1=mem->asm_value(NULL);
		iFUNCTION::emit_asm_ins(string_format("%s %s,DWORD PTR %s",asm_code.c_str(),result_reg->as(4).c_str(),asm_op1.c_str()));
	}else if(op1->is_BIT_FIELD())
	{
		struct REG* reg_new=load_to_reg(op1);
		iFUNCTION::emit_asm_ins(string_format("%s %s,%s",asm_code.c_str(),result_reg->as(4).c_str(),reg_new->as(4).c_str()));
	}else if(op1->is_TEMP())
	{
		struct TEMP* temp=static_cast<struct TEMP*>(op1);
		if(temp->reg!=NULL)
		{
			iFUNCTION::emit_asm_ins(string_format("%s %s,%s",asm_code.c_str(),result_reg->as(4).c_str(),temp->reg->as(4).c_str()));
		}else
		{
			iFUNCTION::emit_asm_ins(string_format("%s %s,DWORD PTR %s",asm_code.c_str(),result_reg->as(4).c_str(),temp->mem->asm_value().c_str()));
		}
	}
	result_reg->unlock();
	return result_reg;
}
static void load_to_floating_st(struct ADDRESS* addr)
{
	if(addr->is_CONST())
	{
		struct CONST* con=static_cast<struct CONST*>(addr);
		iFUNCTION::emit_asm_ins(string_format("fld QWORD PTR %s",new_floating(con)->asm_value().c_str()));
	}else if(addr->is_MEM())
	{
		struct MEM* mem=static_cast<struct MEM*>(addr);
		iFUNCTION::emit_asm_ins(string_format("fld %s %s",address_ptr(mem->type->size).c_str(),mem->asm_value().c_str()));
	}else if(addr->is_TEMP())
	{
		//already cast to floating,so now is in mem
		load_to_floating_st(static_cast<struct TEMP*>(addr)->mem);
	}
}

void IC_ADD::translate()
{
	struct ADDRESS* op0=this->operand0;
	struct ADDRESS* op1=this->operand1;
	if(this->result->type->is_floating_type())
	{
		load_to_floating_st(op0);
		load_to_floating_st(op1);
		iFUNCTION::emit_asm_ins(string_format("faddp st(1),st(0)"));
		this->result->save_floating_st();
	}else
	{
		struct REG* result_reg=asm_standard_binary("add",op0,op1);
		result_reg->set(this->result);
	}
}
void IC_MUL::translate()
{
	struct ADDRESS* op0=this->operand0;
	struct ADDRESS* op1=this->operand1;
	if(this->result->type->is_floating_type())
	{
		load_to_floating_st(op0);
		load_to_floating_st(op1);
		iFUNCTION::emit_asm_ins(string_format("fmulp st(1),st(0)"));
		this->result->save_floating_st();
	}else
	{
		struct REG* result_reg=asm_standard_binary("imul",op0,op1);
		result_reg->set(this->result);
	}
}
void IC_SUB::translate()
{
	struct ADDRESS* op0=this->operand0;
	struct ADDRESS* op1=this->operand1;
	if(this->result->type->is_floating_type())
	{
		load_to_floating_st(op0);
		load_to_floating_st(op1);
		iFUNCTION::emit_asm_ins(string_format("fsubp st(1),st(0)"));
		this->result->save_floating_st();
	}else
	{
		struct REG* result_reg=asm_standard_binary("sub",op0,op1);
		result_reg->set(this->result);
	}
}
static struct REG* asm_integral_div(struct ADDRESS* op0,struct ADDRESS* op1,bool is_mod)
{
//load op0 to result_reg
	struct REG* eax_reg=x86_reg.get_reg("eax");
	struct REG* edx_reg=x86_reg.get_reg("edx");
	load_to_reg(op0,eax_reg);
//applay asm_code on result_reg,op1
	std::string asm_code;
	if(op0->type->is_unsigned() || op1->type->is_unsigned())
	{
		iFUNCTION::emit_asm_ins(string_format("mov edx,0"));
		asm_code="div";
	}else
	{
		iFUNCTION::emit_asm_ins(string_format("cdq"));
		asm_code="idiv";
	}
	struct REG* op1_reg;
	eax_reg->lock();
	edx_reg->lock();
	std::string asm_op1;
	if(op1->is_CONST())
	{
		op1_reg=load_to_reg(op1);
		iFUNCTION::emit_asm_ins(string_format("%s %s",asm_code.c_str(),op1_reg->as(4).c_str()));
	}else if(op1->is_VAR())
	{
		op1_reg=load_to_reg(op1);
		iFUNCTION::emit_asm_ins(string_format("%s %s",asm_code.c_str(),op1_reg->as(4).c_str()));
	}else if(op1->is_MEM())
	{
		struct MEM* mem=static_cast<struct MEM*>(op1);
		asm_op1=mem->asm_value(NULL);
		iFUNCTION::emit_asm_ins(string_format("%s DWORD PTR %s",asm_code.c_str(),asm_op1.c_str()));
	}else if(op1->is_BIT_FIELD())
	{
		op1_reg=load_to_reg(op1);
		iFUNCTION::emit_asm_ins(string_format("%s %s",asm_code.c_str(),op1_reg->as(4).c_str()));
	}else if(op1->is_TEMP())
	{
		struct TEMP* temp=static_cast<struct TEMP*>(op1);
		if(temp->reg!=NULL)
		{
			iFUNCTION::emit_asm_ins(string_format("%s %s",asm_code.c_str(),temp->reg->as(4).c_str()));
		}else
		{
			asm_op1=temp->mem->asm_value(NULL);
			iFUNCTION::emit_asm_ins(string_format("%s DWORD PTR %s",asm_code.c_str(),asm_op1.c_str()));
		}
	}
	eax_reg->unlock();
	edx_reg->unlock();
	if(is_mod)
		return edx_reg;
	else
		return eax_reg;
}
void IC_DIV::translate()
{
	struct ADDRESS* op0=this->operand0;
	struct ADDRESS* op1=this->operand1;
	if(this->result->type->is_floating_type())
	{
		load_to_floating_st(op0);
		load_to_floating_st(op1);
		iFUNCTION::emit_asm_ins(string_format("fdivp st(1),st(0)"));
		this->result->save_floating_st();
	}else
	{
		struct REG* result_reg=asm_integral_div(op0,op1,false);
		result_reg->set(this->result);
	}
}

void IC_MOD::translate()
{
	struct ADDRESS* op0=this->operand0;
	struct ADDRESS* op1=this->operand1;
	struct REG* result_reg=asm_integral_div(op0,op1,true);
	result_reg->set(this->result);
}
void IC_LSHIFT::translate()
{
	struct ADDRESS* op0=this->operand0;
	struct ADDRESS* op1=this->operand1;
	struct REG* ecx_reg=x86_reg.get_reg("ecx");
	load_to_reg(op1,ecx_reg);
	ecx_reg->lock();
	struct REG* result_reg=load_to_reg(op0);
	iFUNCTION::emit_asm_ins(string_format("shl %s,cl",result_reg->as(4).c_str()));
	ecx_reg->unlock();
	result_reg->set(this->result);
}
void IC_RSHIFT::translate()
{
	struct ADDRESS* op0=this->operand0;
	struct ADDRESS* op1=this->operand1;
	struct REG* ecx_reg=x86_reg.get_reg("ecx");
	load_to_reg(op1,ecx_reg);
	ecx_reg->lock();
	struct REG* result_reg=load_to_reg(op0);
	if(op0->type->is_unsigned() || op1->type->is_unsigned())
		iFUNCTION::emit_asm_ins(string_format("shr %s,cl",result_reg->as(4).c_str()));
	else
		iFUNCTION::emit_asm_ins(string_format("sar %s,cl",result_reg->as(4).c_str()));
	ecx_reg->unlock();
	result_reg->set(this->result);
}
void IC_BITAND::translate()
{
	struct ADDRESS* op0=this->operand0;
	struct ADDRESS* op1=this->operand1;
	struct REG* result_reg=asm_standard_binary("and",op0,op1);
	result_reg->set(this->result);
}
void IC_BITXOR::translate()
{
	struct ADDRESS* op0=this->operand0;
	struct ADDRESS* op1=this->operand1;
	struct REG* result_reg=asm_standard_binary("xor",op0,op1);
	result_reg->set(this->result);
}
void IC_BITOR::translate()
{
	struct ADDRESS* op0=this->operand0;
	struct ADDRESS* op1=this->operand1;
	struct REG* result_reg=asm_standard_binary("or",op0,op1);
	result_reg->set(this->result);
}
void IC_COMP::translate()
{
	struct ADDRESS* op0=this->operand0;
	struct REG* result_reg=load_to_reg(op0);
	iFUNCTION::emit_asm_ins(string_format("not %s",result_reg->as(4).c_str()));
	result_reg->set(this->result);
}
void IC_NEG::translate()
{
	struct ADDRESS* op0=this->operand0;
	struct REG* result_reg=load_to_reg(op0);
	iFUNCTION::emit_asm_ins(string_format("neg %s",result_reg->as(4).c_str()));
	result_reg->set(this->result);
}


static void reg_to_floating_st(bool is_unsigned,struct REG* reg)
{
	if(is_unsigned)
	{
		struct MEM* mem=MEM::local_temp(TYPE::basic_type("float64"));
		iFUNCTION::emit_asm_ins(string_format("mov DWORD PTR %s,%s",mem->asm_value().c_str(),reg->as(4).c_str()));
		iFUNCTION::emit_asm_ins(string_format("mov DWORD PTR %s,0",mem->var_asm_value(4).c_str()));
		iFUNCTION::emit_asm_ins(string_format("fild QWORD PTR %s",mem->asm_value().c_str()));
	}else
	{
		struct MEM* mem=MEM::local_temp(TYPE::basic_type("float32"));
		iFUNCTION::emit_asm_ins(string_format("mov DWORD PTR %s,%s",mem->asm_value().c_str(),reg->as(4).c_str()));
		iFUNCTION::emit_asm_ins(string_format("fild DWORD PTR %s",mem->asm_value().c_str()));
	}
}
static void CVT_integral_to_floating_st(struct ADDRESS* addr)
{
	if(addr->is_VAR())
	{
		struct REG* new_reg=load_to_reg(addr);
		reg_to_floating_st(addr->type->is_unsigned(),new_reg);
	}else if(addr->is_MEM())
	{
		struct REG* new_reg;
		if(static_cast<struct BASIC_TYPE*>(addr->type)->is_uint32())
		{
			new_reg=load_to_reg(addr);
			reg_to_floating_st(true,new_reg);
		}else
		{
			if(addr->type->size < 4)
			{
				new_reg=upcast(4,addr);
				reg_to_floating_st(false,new_reg);
			}else//int32
			{
				struct MEM* mem=static_cast<struct MEM*>(addr);
				iFUNCTION::emit_asm_ins(string_format("fild DWORD PTR %s",mem->asm_value().c_str()));
			}
		}
	}else if(addr->is_BIT_FIELD())
	{
		if(addr->type->size < 4)
			reg_to_floating_st(false,upcast(4,addr));
		else
			reg_to_floating_st(addr->type->is_unsigned(),load_to_reg(addr));
	}
	else if(addr->is_TEMP())
	{
		struct TEMP* temp=static_cast<struct TEMP*>(addr);
		if(temp->reg!=NULL)
		{
			if(addr->type->size < 4)
			{
				struct REG*new_reg=upcast(4,temp);
				reg_to_floating_st(false,new_reg);
			}else//int32 or uint32
			{
				reg_to_floating_st(static_cast<struct BASIC_TYPE*>(temp->type)->is_uint32(),temp->reg);
			}
		}else
		{
			CVT_integral_to_floating_st(temp->mem);
		}
	}
}
static struct REG* CVT_floating_to_register(struct ADDRESS* addr)
{
	struct REG* result_reg;
	if(addr->is_MEM())
	{
		result_reg=x86_reg.get_reg();
		struct MEM* mem=static_cast<struct MEM*>(addr);
		if(static_cast<struct BASIC_TYPE*>(addr->type)->is_float32())
		{
			iFUNCTION::emit_asm_ins(string_format("cvttss2si %s,DWORD PTR %s",result_reg->as(4).c_str(),mem->asm_value(result_reg).c_str()));
		}else
		{
			iFUNCTION::emit_asm_ins(string_format("cvttsd2si %s,QWORD PTR %s",result_reg->as(4).c_str(),mem->asm_value(result_reg).c_str()));
		}
	}else if(addr->is_TEMP())
	{//already be cast to flaoting ,so now in mem.
		result_reg=CVT_floating_to_register(static_cast<struct TEMP*>(addr)->mem);
	}
	return result_reg;
}
static struct REG* CVT_floating_to_register_uint32(struct ADDRESS* addr)
{
	load_to_floating_st(addr);
	struct REG* result_reg=x86_reg.get_reg();//also using as free register for temperal value.
	struct MEM* save_control_word=MEM::local_temp(TYPE::basic_type("uint16"));
	iFUNCTION::emit_asm_ins(string_format("fnstcw WORD PTR %s",save_control_word->asm_value().c_str()));
	iFUNCTION::emit_asm_ins(string_format("movzx %s,WORD PTR %s",result_reg->as(4).c_str(),save_control_word->asm_value().c_str()));
	iFUNCTION::emit_asm_ins(string_format("or %s,3072",result_reg->as(4).c_str()));
	struct MEM* control_word_of_truncate=MEM::local_temp(TYPE::basic_type("uint32"));
	iFUNCTION::emit_asm_ins(string_format("mov DWORD PTR %s,%s",control_word_of_truncate->asm_value().c_str(),result_reg->as(4).c_str()));
	iFUNCTION::emit_asm_ins(string_format("fldcw WORD PTR %s",control_word_of_truncate->asm_value().c_str()));
	struct MEM* truncated_result=MEM::local_temp(TYPE::basic_type("float64"));
	iFUNCTION::emit_asm_ins(string_format("fistp QWORD PTR %s",truncated_result->asm_value().c_str()));
	iFUNCTION::emit_asm_ins(string_format("fldcw WORD PTR %s",save_control_word->asm_value().c_str()));
	iFUNCTION::emit_asm_ins(string_format("mov %s,DWORD PTR %s",result_reg->as(4).c_str(),truncated_result->asm_value().c_str()));
	return result_reg;
}
void IC_CVT::translate()
{
	if( this->result->type->is_floating_type() || this->operand0->type->is_floating_type())
	{
		if(!this->operand0->type->is_floating_type())
		{//integral to floating
			CVT_integral_to_floating_st(this->operand0);
			this->result->save_floating_st();
		}else if(!this->result->type->is_floating_type())
		{//floating to integral
			struct REG* result_reg;
			if(static_cast<struct BASIC_TYPE*>(this->result->type)->is_uint32())
			{
				result_reg=CVT_floating_to_register_uint32(this->operand0);
			}else
			{
				result_reg=CVT_floating_to_register(this->operand0);
			}
			result_reg->set(this->result);
		}else
		{//floating to floating
			load_to_floating_st(this->operand0);
			this->result->save_floating_st();
		}
	}else
	{
		struct REG* result_reg;
		if(this->result->type->size > this->operand0->type->size)
		{//upcast
			result_reg=upcast(this->result->type->size,this->operand0);
			result_reg->add(this->result);
		}else if(this->result->type->size < this->operand0->type->size)
		{//downcast
			result_reg=downcast(this->result->type->size,this->operand0);
			result_reg->add(this->result);
		}
	}
}
static void save_integral_from_register(struct ADDRESS* addr,struct REG* reg)
{
	reg->lock();	
	if(addr->is_TEMP())
	{
		struct TEMP* temp=static_cast<struct TEMP*>(addr);
		reg->add(temp);
	}else if(addr->is_MEM())
	{
		struct MEM* mem=static_cast<struct MEM*>(addr);
		std::string asm_tgt=mem->asm_value();
		unsigned int tgt_width=addr->type->size;
		iFUNCTION::emit_asm_ins(string_format("mov %s %s,%s",
			address_ptr(tgt_width).c_str(),
			asm_tgt.c_str(),
			reg->as(tgt_width).c_str()));
	}else if(addr->is_BIT_FIELD())
	{
		struct BIT_FIELD* bf=static_cast<struct BIT_FIELD*>(addr);
		bf->store_from_reg(reg);
	}
	reg->unlock();
}
static void save_floating_from_floating_st(struct ADDRESS* addr)
{
	if(addr->is_TEMP())
	{
		static_cast<struct TEMP*>(addr)->save_floating_st();
	}else if(addr->is_MEM())
	{
		struct MEM* mem=static_cast<struct MEM*>(addr);
		std::string asm_tgt=mem->asm_value();
		unsigned int tgt_width=addr->type->size;
		iFUNCTION::emit_asm_ins(string_format("fstp %s %s",
			address_ptr(tgt_width).c_str(),
			asm_tgt.c_str()));
	}
}
void IC_MOVE::translate()
{
	struct ADDRESS* src=this->operand1;
	struct ADDRESS* tgt=this->operand0;
	if(tgt->type->is_floating_type())
	{
		load_to_floating_st(src);
		save_floating_from_floating_st(tgt);
	}else
	{
		struct REG* result_reg=load_to_reg(src);
		save_integral_from_register(tgt,result_reg);
	}
}
void IC_MOVEM::translate()
{
	struct ADDRESS* src=this->operand1;
	struct ADDRESS* tgt=this->operand0;
	struct REG* edi_reg=x86_reg.get_reg("edi");
	struct REG* esi_reg=x86_reg.get_reg("esi");

	load_to_reg(tgt,edi_reg);
	load_to_reg(src,esi_reg);
	x86_reg.get_reg("ecx");
	iFUNCTION::emit_asm_ins(string_format("mov ecx,%u",this->size));
	iFUNCTION::emit_asm_ins(string_format("rep movsb"));
}
void IC_CLEAR::translate()
{
	struct ADDRESS* addr=this->operand0;

	iFUNCTION::emit_asm_ins(string_format("push %u",this->size));
	iFUNCTION::emit_asm_ins(string_format("push 0"));
	struct REG* reg=load_to_reg(addr);
	iFUNCTION::emit_asm_ins(string_format("push %s",reg->as(4).c_str()));
	iFUNCTION::emit_asm_ins(string_format("call _memset"));
	iFUNCTION::emit_asm_ins(string_format("add esp,12"));
}
static std::string asm_jump_code(std::string op,bool is_unsigned)
{
	std::string code;
	if(op=="==")
		code="je";
	else if(op=="!=")
		code="jne";
	else if(op=="<")
		code=is_unsigned?"jb":"jl";
	else if(op==">=")
		code=is_unsigned?"jae":"jge";
	else if(op==">")
		code=is_unsigned?"ja":"jg";
	else if(op=="<=")
		code=is_unsigned?"jbe":"jle";
	else
		assert(0);
	return code;
}
static std::string asm_set_code(std::string op,bool is_unsigned)
{
	std::string code;
	if(op=="==")
		code="sete";
	else if(op=="!=")
		code="setne";
	else if(op=="<")
		code=is_unsigned?"setb":"setl";
	else if(op==">=")
		code=is_unsigned?"setae":"setge";
	else if(op==">")
		code=is_unsigned?"seta":"setg";
	else if(op=="<=")
		code=is_unsigned?"setbe":"setle";
	else
		assert(0);
	return code;
}
void IJUMP::translate()
{
	struct ADDRESS* op0=this->operand0;
	struct ADDRESS* op1=this->operand1;
	std::string jmp_code;
	if(op0->type->is_floating_type() || op1->type->is_floating_type())
	{
		load_to_floating_st(op1);
		load_to_floating_st(op0);
		if(this->op=="==" || this->op=="!=")
		{
			iFUNCTION::emit_asm_ins(string_format("fucomip ST(0),ST(1)"));
			iFUNCTION::emit_asm_ins(string_format("fstp ST(0)"));//discard result.
			x86_reg.get_reg("eax");
			iFUNCTION::emit_asm_ins(string_format("lahf"));//status to ah
			iFUNCTION::emit_asm_ins(string_format("test ah,68"));//100100b,zf,pf.

			jmp_code=this->op=="=="?"jnp":"jp";
		}else
		{
			iFUNCTION::emit_asm_ins(string_format("fcomip ST(0),ST(1)"));
			iFUNCTION::emit_asm_ins(string_format("fstp ST(0)"));//discard result.
			jmp_code=asm_jump_code(this->op,true);
		}
	}else
	{
		asm_standard_binary("cmp",op0,op1);
		jmp_code=asm_jump_code(this->op,op0->type->is_unsigned() || op1->type->is_unsigned());
	}
	//dec ref then spill reg.
	this->dec_ref();
	x86_reg.spill_all();
	iFUNCTION::emit_asm_ins(string_format("%s %s",jmp_code.c_str(),this->label->asm_name().c_str()));
}
void ISET::translate()
{
	struct ADDRESS* op0=this->operand0;
	struct ADDRESS* op1=this->operand1;
	std::string code;
	if(op0->type->is_floating_type() || op1->type->is_floating_type())
	{
		load_to_floating_st(op1);//st(1)
		load_to_floating_st(op0);//st(0)
		if(this->op=="==" || this->op=="!=")
		{
			iFUNCTION::emit_asm_ins(string_format("fucomip ST(0),ST(1)"));
			iFUNCTION::emit_asm_ins(string_format("fstp ST(0)"));//discard result.
			x86_reg.get_reg("eax");
			iFUNCTION::emit_asm_ins(string_format("lahf"));//status to ah
			iFUNCTION::emit_asm_ins(string_format("test ah,68"));//100100b,zf,pf.

			code=this->op=="=="?"setnp":"setp";
		}else
		{
			iFUNCTION::emit_asm_ins(string_format("fcomip ST(0),ST(1)"));
			iFUNCTION::emit_asm_ins(string_format("fstp ST(0)"));//discard result.
			code=asm_set_code(this->op,true);
		}
	}else
	{
		asm_standard_binary("cmp",op0,op1);
		code=asm_set_code(this->op,op0->type->is_unsigned() || op1->type->is_unsigned());
	}
	struct REG* result_reg=x86_reg.get_reg();
	iFUNCTION::emit_asm_ins(string_format("mov %s,0",result_reg->as(4).c_str()));
	iFUNCTION::emit_asm_ins(string_format("%s %s",code.c_str(),result_reg->as(1).c_str()));
	result_reg->set(this->result);
}
void JUMP::translate()
{
	//dec ref then spill reg.
	this->dec_ref();
	x86_reg.spill_all();
	iFUNCTION::emit_asm_ins(string_format("JMP %s",this->label->asm_name().c_str()));
}

void RET::translate()
{
	struct ADDRESS* op0=this->operand0;
	if(op0->type->is_struct_or_union())
	{
		struct ADDRESS* addr=MEM::addr_of(op0);
		struct REG* esi_reg=x86_reg.get_reg("esi");
		load_to_reg(addr,esi_reg);
		iFUNCTION::emit_asm_ins(string_format("mov edi,(%u)[ebp]",FIRST_PARAMETER_OFFSET));
		x86_reg.get_reg("ecx");
		iFUNCTION::emit_asm_ins(string_format("mov ecx,%u",op0->type->size));
		iFUNCTION::emit_asm_ins(string_format("rep movsb"));
	}else
	{
		if(this->operand0->type->is_floating_type())
		{
			load_to_floating_st(this->operand0);
		}else
		{
			struct REG* result_reg=x86_reg.get_reg("eax");
			load_to_reg(this->operand0,result_reg);
		}
	}
}
static struct REG* load_to_reg_zx(struct ADDRESS* op)
{
	struct REG* result_reg;
	std::string asm_code;
	unsigned int source_width=op->type->size;
	bool extend_mode=source_width<4;
	asm_code=extend_mode?"movzx":"mov";
	if(op->is_CONST())
	{
		result_reg=load_to_reg(op);
	}else if(op->is_VAR())
	{
		result_reg=load_to_reg(op);
	}else if(op->is_MEM())
	{
		result_reg=x86_reg.get_reg();
		struct MEM* mem=static_cast<struct MEM*>(op);
		std::string asm_op=mem->asm_value(result_reg);
		iFUNCTION::emit_asm_ins(string_format("%s %s,%s %s",
			asm_code.c_str(),
			result_reg->as(4).c_str(),
			address_ptr(source_width).c_str(),
			asm_op.c_str()));
	}else if(op->is_BIT_FIELD())
	{
		result_reg=load_to_reg(op);
	}else if(op->is_TEMP())
	{
		struct TEMP* temp=static_cast<struct TEMP*>(op);
		std::string asm_op;
		if(temp->reg!=NULL)
		{
			result_reg=temp->reg;
			if(extend_mode)
			{
				iFUNCTION::emit_asm_ins(string_format("%s %s,%s",
					asm_code.c_str(),
					result_reg->as(4).c_str(),
					result_reg->as(source_width).c_str()));
			}
		}else
		{
			result_reg=load_to_reg_zx(temp->mem);
		}
	}
	return result_reg;
}
void PARAM::translate()
{
	struct ADDRESS* op0=this->operand0;
	if(op0->type->is_struct_or_union())
	{
		struct ADDRESS* addr=MEM::addr_of(op0);
		struct REG* esi_reg=x86_reg.get_reg("esi");
		load_to_reg(addr,esi_reg);
		iFUNCTION::emit_asm_ins(string_format("sub esp,%u",ALIGN(op0->type->size,PARAMETER_ALIGN)));
		iFUNCTION::emit_asm_ins(string_format("mov edi,esp"));
		x86_reg.get_reg("ecx");
		iFUNCTION::emit_asm_ins(string_format("mov ecx,%u",op0->type->size));
		iFUNCTION::emit_asm_ins(string_format("rep movsb"));
	}else
	{
		if(op0->type->is_floating_type())
		{
			iFUNCTION::emit_asm_ins(string_format("sub esp,%u",op0->type->size));
			load_to_floating_st(op0);
			iFUNCTION::emit_asm_ins(string_format("fstp %s [esp]",address_ptr(op0->type->size).c_str()));
		}else
		{
			struct REG* result_reg=load_to_reg_zx(op0);
			iFUNCTION::emit_asm_ins(string_format("push %s",result_reg->as(4).c_str()));
		}
	}
}
static void call_by_reg(struct CALL* ins,struct REG* reg)
{
	//dec ref then spill reg.
	ins->dec_ref();
	x86_reg.spill_all();
	iFUNCTION::emit_asm_ins(string_format("call %s",reg->as(4).c_str()));
}
static void call_by_mem(struct CALL* ins,struct MEM* mem)
{
	std::string asm_op=mem->asm_value();
	//dec ref then spill reg.
	ins->dec_ref();
	x86_reg.spill_all();
	iFUNCTION::emit_asm_ins(string_format("call DWORD PTR %s",asm_op.c_str()));
}
static void call_by_function(struct CALL* ins,struct IDENTIFIER* function)
{
	//dec ref then spill reg.
	ins->dec_ref();
	x86_reg.spill_all();
	iFUNCTION::emit_asm_ins(string_format("call %s",get_access_name(function).c_str()));
}
void CALL::translate()
{
	struct ADDRESS* op0=this->operand0;
	if(op0->is_CONST())
	{
		call_by_reg(this,load_to_reg(op0));
	}else if(op0->is_VAR())
	{
		struct VAR* var=static_cast<struct VAR*>(op0);
		if(var->var->type->is_function())
			call_by_function(this,var->var);
		else
			call_by_reg(this,load_to_reg(op0));
	}else if(op0->is_MEM())
	{
		struct MEM* mem=static_cast<struct MEM*>(op0);
		call_by_mem(this,mem);
	}else if(op0->is_BIT_FIELD())
	{
		call_by_reg(this,load_to_reg(op0));
	}else if(op0->is_TEMP())
	{
		struct TEMP* temp=static_cast<struct TEMP*>(op0);
		if(temp->reg!=NULL)
			call_by_reg(this,temp->reg);
		else
			call_by_mem(this,temp->mem);
	}

	//balance stack
	iFUNCTION::emit_asm_ins(string_format("add esp,%u",this->stack_size));
	if(this->result->type->is_struct_or_union())
	{
		//nothing
	}else
	{
		if(this->result->type->is_floating_type())
		{
			this->result->save_floating_st();
		}else
		{
			x86_reg.get_reg("eax")->add(this->result);
		}
	}
}