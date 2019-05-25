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
std::string get_access_name(struct IDENTIFIER* identifier)
{
	if(identifier->storage=="static" && identifier->linkage!="none")
	{
		return string_format("_%s",identifier->name.c_str());
	}else
	{
		return string_format("_%s_%d",identifier->name.c_str(),identifier->id);
	}
}
std::string CONST::asm_value()
{
	return toString();
}
std::string VAR::asm_value_offset(unsigned int the_offset)
{
	std::string v;
	std::string access_name=get_access_name(this->var);
	if(the_offset==0)
	{
		if(this->var->storage=="static")
			v=string_format("%s",access_name.c_str());
		else
			v=string_format("%s[ebp]",access_name.c_str());
	}else
	{
		if(this->var->storage=="static")
			v=string_format("%s+%u",access_name.c_str(),the_offset);
		else
			v=string_format("%s[ebp+%u]",access_name.c_str(),the_offset);
	}
	return v;
}
std::string VAR::asm_value()
{
	return asm_value_offset(this->offset);
}
void VAR::load_to_reg(struct REG* reg)
{
	if(this->var->storage=="static")
		iFUNCTION::emit_asm_ins(string_format("lea %s,%s",reg->as(4).c_str(),this->asm_value().c_str()));
	else
		iFUNCTION::emit_asm_ins(string_format("lea %s,DWORD PTR %s",reg->as(4).c_str(),this->asm_value().c_str()));
}
std::string MEM::asm_value(struct REG* option_reg)
{
	std::string v;
	if(this->addr->is_CONST())
	{
		struct CONST* con=static_cast<struct CONST*>(this->addr);
		v=string_format("[%s]",con->asm_value().c_str());
	}else if(this->addr->is_VAR())
	{
		struct VAR* var=static_cast<struct VAR*>(this->addr);
		v=var->asm_value();
	}else if(this->addr->is_TEMP())
	{
		struct TEMP* temp=static_cast<struct TEMP*>(this->addr);
		if(temp->reg==NULL)
		{
			if(option_reg==NULL)
				option_reg=x86_reg.get_reg();
			iFUNCTION::emit_asm_ins(string_format("mov %s,DWORD PTR %s",option_reg->as(4).c_str(),temp->mem->asm_value().c_str()));
			v=string_format("[%s]",option_reg->as(4).c_str());
		}else
			v=string_format("[%s]",temp->reg->as(4).c_str());
	}
	return v;
}
std::string MEM::var_asm_value(unsigned int offset)
{
	std::string v;
	assert(this->addr->is_VAR());
	struct VAR* var=static_cast<struct VAR*>(this->addr);
	v=var->asm_value_offset(offset);
	return v;
}

std::string BIT_FIELD::asm_value(struct REG* option_reg)
{
	std::string v;
	if(this->addr->is_CONST())
	{
		struct CONST* con=static_cast<struct CONST*>(this->addr);
		v=string_format("[%s]",con->asm_value().c_str());
	}else if(this->addr->is_VAR())
	{
		struct VAR* var=static_cast<struct VAR*>(this->addr);
		v=var->asm_value();
	}else if(this->addr->is_TEMP())
	{
		struct TEMP* temp=static_cast<struct TEMP*>(this->addr);
		if(temp->reg==NULL)
		{
			if(option_reg==NULL)
				option_reg=x86_reg.get_reg();
			iFUNCTION::emit_asm_ins(string_format("mov %s,DWORD PTR %s",option_reg->as(4).c_str(),temp->mem->asm_value().c_str()));
			v=string_format("[%s]",option_reg->as(4).c_str());
		}else
			v=string_format("[%s]",temp->reg->as(4).c_str());
	}
	return v;
}
void BIT_FIELD::store_from_reg(struct REG* reg)
{
	size_t prefix_n=this->bit_offset;
	unsigned int bmask=(1<<width)-1;
	unsigned int bmask_clear=~(bmask<<this->bit_offset);

	reg->lock();
	iFUNCTION::emit_asm_ins(string_format("and %s,%d\t;%08xH",reg->as(this->type->size).c_str(),bmask,bmask));
	iFUNCTION::emit_asm_ins(string_format("shl %s,%d",reg->as(this->type->size).c_str(),prefix_n));
	struct REG* reg2=x86_reg.get_reg();
	reg2->lock();
	iFUNCTION::emit_asm_ins(string_format("mov %s,%d\t;%08xH",reg2->as(4).c_str(),bmask_clear,bmask_clear));
	iFUNCTION::emit_asm_ins(string_format("and %s,%s %s",reg2->as(this->type->size).c_str(),
										address_ptr(this->type->size).c_str(),this->asm_value().c_str()));


	iFUNCTION::emit_asm_ins(string_format("or %s,%s",reg2->as(this->type->size).c_str(),reg->as(this->type->size).c_str()));

	iFUNCTION::emit_asm_ins(string_format("mov %s %s,%s",address_ptr(this->type->size).c_str(),this->asm_value(reg).c_str(),
										reg2->as(this->type->size).c_str()));
	reg->unlock();
	reg2->unlock();
}
void BIT_FIELD::load_to_reg(struct REG* reg)
{
	iFUNCTION::emit_asm_ins(string_format("mov %s,%s %s",reg->as(this->type->size).c_str(),
		address_ptr(this->type->size).c_str(),this->asm_value(reg).c_str()));

	size_t suffix_n=this->type->size*8-(this->bit_offset+this->width);
	size_t prefix_n=this->bit_offset;
	unsigned int bmask=(1<<width)-1;
	if(this->is_unsigned)
	{
		iFUNCTION::emit_asm_ins(string_format("shr %s,%d",reg->as(this->type->size).c_str(),prefix_n));
		iFUNCTION::emit_asm_ins(string_format("and %s,%d\t;%08xH",reg->as(this->type->size).c_str(),bmask,bmask));
	}else
	{
		iFUNCTION::emit_asm_ins(string_format("shl %s,%d",reg->as(this->type->size).c_str(),suffix_n));
		iFUNCTION::emit_asm_ins(string_format("sar %s,%d",reg->as(this->type->size).c_str(),suffix_n+prefix_n));
	}
}
void TEMP::save_floating_st()
{
	if(this->mem==NULL)
		this->mem=MEM::local_temp(this->type);
	
	iFUNCTION::emit_asm_ins(string_format("fstp %s %s",address_ptr(this->type->size).c_str(),
		this->mem->asm_value().c_str()));
}
void TEMP::save(struct REG* r)
{
	if(this->mem==NULL)
		this->mem=MEM::local_temp(this->type);
	
	iFUNCTION::emit_asm_ins(string_format("mov %s %s,%s",address_ptr(this->type->size).c_str(),this->mem->asm_value().c_str(),r->as(this->type->size).c_str()));
}