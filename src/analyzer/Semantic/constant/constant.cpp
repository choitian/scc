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
#include "..\semantic_analyzer.h"
#include "..\..\Syntax_Tree\syntax_tree.h"
#include "..\..\Syntax_Tree\syntax_tree_decl.h"
#include "..\..\Syntax_Tree\syntax_tree_stmt.h"
#include "..\..\Syntax_Tree\syntax_tree_exp.h"
#include "..\Symbol_Table\type.h"
#include "..\Symbol_Table\identifier.h"
#include "..\Symbol_Table\scope.h"

#include "..\expression\IC.h"
#include "constant.h"

void CONST::cast_to(struct TYPE* type)
{
	std::string from=this->type->op;
	std::string to=type->op;
	if(from!=to)
	{
		if(to=="int8")
		{
			cast_from<char>(&this->value.int8);
		}else if(to=="uint8")
		{
			cast_from<unsigned char>(&this->value.uint8);
		}else if(to=="int16")
		{
			cast_from<short int>(&this->value.int16);
		}else if(to=="uint16")
		{
			cast_from<unsigned short int>(&this->value.uint16);
		}else if(to=="int32")
		{
			cast_from<long int>(&this->value.int32);
		}else if(to=="uint32")
		{
			cast_from<unsigned long int>(&this->value.uint32);
		}else if(to=="float32")
		{
			cast_from<float>(&this->value.float32);
		}else if(to=="float64")
		{
			cast_from<double>(&this->value.float64);
		}else if(to=="PTR")
		{
			cast_from<unsigned long int>(&this->value.uint32);
		}else
			assert(0);//never here.
		this->type=type;
	}
}
int CONST::as_int32()
{
	int v;
	std::string type_code=this->type->op;
	if(type_code=="int8")
	{
		v=this->value.int8;
	}else if(type_code=="uint8")
	{
		v=this->value.uint8;
	}else if(type_code=="int16")
	{
		v=this->value.int16;		
	}else if(type_code=="uint16")
	{
		v=this->value.uint16;						
	}else if(type_code=="int32")
	{
		v=this->value.int32;						
	}else if(type_code=="uint32")
	{
		v=(int)this->value.uint32;											
	}else if(type_code=="float32")
	{
		v=(int)this->value.float32;									
	}else if(type_code=="float64")
	{
		v=(int)this->value.float64;						
	}else
		v=(int)this->value.uint32;	
	return v;
}
unsigned int CONST::as_uint32()
{
	unsigned int v;
	std::string type_code=this->type->op;
	if(type_code=="int8")
	{
		v=(unsigned int)this->value.int8;
	}else if(type_code=="uint8")
	{
		v=(unsigned int)this->value.uint8;
	}else if(type_code=="int16")
	{
		v=(unsigned int)this->value.int16;		
	}else if(type_code=="uint16")
	{
		v=(unsigned int)this->value.uint16;						
	}else if(type_code=="int32")
	{
		v=(unsigned int)this->value.int32;						
	}else if(type_code=="uint32")
	{
		v=(unsigned int)this->value.uint32;											
	}else if(type_code=="float32")
	{
		v=(unsigned int)this->value.float32;									
	}else if(type_code=="float64")
	{
		v=(unsigned int)this->value.float64;						
	}else
		v=(unsigned int)this->value.uint32;	
	return v;
}
struct CONST* CONST::constant_expression_2_const(struct exp* expr)
{
	struct CONST* con=NULL;
	bool discard_ins_old=iFUNCTION::current_definition->discard_ins;
	iFUNCTION::current_definition->discard_ins=true;
	expr->check();
	if(expr->addr->is_CONST())
	{
		con=static_cast<struct CONST*>(expr->addr);
	}
	iFUNCTION::current_definition->discard_ins=discard_ins_old;
	return con;
}
bool CONST::is_zero()
{
	std::string type_code=this->type->op;
	if(type_code=="int8")
	{
		return 0==this->value.int8;
	}else if(type_code=="uint8")
	{
		return 0==this->value.uint8;
	}else if(type_code=="int16")
	{
		return 0==this->value.int16;		
	}else if(type_code=="uint16")
	{
		return 0==this->value.uint16;						
	}else if(type_code=="int32")
	{
		return 0==this->value.int32;						
	}else if(type_code=="uint32")
	{
		return 0==this->value.uint32;											
	}else if(type_code=="float32")
	{
		return 0.0==this->value.float32;									
	}else if(type_code=="float64")
	{
		return 0.0==this->value.float64;						
	}else
		return 0==this->value.uint32;
	return false;
}
bool CONST::is_one()
{
	std::string type_code=this->type->op;
	if(type_code=="int8")
	{
		return 1==this->value.int8;
	}else if(type_code=="uint8")
	{
		return 1==this->value.uint8;
	}else if(type_code=="int16")
	{
		return 1==this->value.int16;		
	}else if(type_code=="uint16")
	{
		return 1==this->value.uint16;						
	}else if(type_code=="int32")
	{
		return 1==this->value.int32;						
	}else if(type_code=="uint32")
	{
		return 1==this->value.uint32;											
	}else if(type_code=="float32")
	{
		return false;									
	}else if(type_code=="float64")
	{
		return false;						
	}else
		return 1==this->value.uint32;
}


template <typename TYPE>
bool add_will_overflow_max(const TYPE& left, const TYPE& right)
{
	const TYPE& type_Max = std::numeric_limits<TYPE>::max();
    return type_Max -  right <left;
}
template <typename TYPE>
bool add_will_overflow_min(const TYPE& left, const TYPE& right)
{
	const TYPE& type_Min = std::numeric_limits<TYPE>::min();
    if( type_Min - right > left)
		return true;
	return false;
}
struct CONST* IC_ADD::constant_fold(struct CONST* fst,struct CONST* snd,bool *overflow)
{
	struct TYPE* type=usual_arithmetic_conversion(fst->type,snd->type);
	std::string common=type->op;
	bool overflow_=false;
	fst->cast_to(type);
	snd->cast_to(type);
	if(common=="int32")
	{
		if(fst->value.int32 >0 && snd->value.int32 >0 &&
			add_will_overflow_max<long int>(fst->value.int32,snd->value.int32))
			overflow_=true;
		if(fst->value.int32 <0 && snd->value.int32 <0 &&
			add_will_overflow_min<long int>(fst->value.int32,snd->value.int32))
			overflow_=true;
		fst->value.int32 +=snd->value.int32;	
	}else if(common=="uint32")
	{
		if(add_will_overflow_max<unsigned long int>(fst->value.uint32,snd->value.uint32))
			overflow_=true;
		fst->value.uint32 += snd->value.uint32;	
	}else if(common=="float32")
	{
		fst->value.float32 += snd->value.float32;			
	}else if(common=="float64")
	{
		fst->value.float64 += snd->value.float64;					
	}
	if(overflow!=NULL)
	{
		*overflow=overflow_;
	}
	return fst;
}


template <typename TYPE>
bool sub_will_overflow_max(const TYPE& left, const TYPE& right)
{
	const TYPE& type_Max = std::numeric_limits<TYPE>::max();
    return type_Max +  right< left;
}
template <typename TYPE>
bool sub_will_overflow_min(const TYPE& left, const TYPE& right)
{
	const TYPE& type_Min = std::numeric_limits<TYPE>::min();
    if( type_Min + right > left)
		return true;
	return false;
}
struct CONST* IC_SUB::constant_fold(struct CONST* fst,struct CONST* snd,bool *overflow)
{
	struct TYPE* type=usual_arithmetic_conversion(fst->type,snd->type);
	std::string common=type->op;
	bool overflow_=false;
	fst->cast_to(type);
	snd->cast_to(type);
	if(common=="int32")
	{
		if(fst->value.int32 >0 && snd->value.int32 <0 &&
			sub_will_overflow_max<long int>(fst->value.int32,snd->value.int32))
			overflow_=true;
		if(fst->value.int32 <0 && snd->value.int32 >0 &&
			sub_will_overflow_min<long int>(fst->value.int32,snd->value.int32))
			overflow_=true;
		fst->value.int32 -=snd->value.int32;	
	}else if(common=="uint32")
	{
		if(add_will_overflow_max<unsigned long int>(fst->value.uint32,snd->value.uint32))
			overflow_=true;
		fst->value.uint32 -= snd->value.uint32;	
	}else if(common=="float32")
	{
		fst->value.float32 -= snd->value.float32;			
	}else if(common=="float64")
	{
		fst->value.float64 -= snd->value.float64;					
	}
	if(overflow!=NULL)
	{
		*overflow=overflow_;
	}
	return fst;
}


template <typename TYPE>
bool mul_will_overflow_max(const TYPE left, const TYPE right)
{
	const TYPE& type_Max = std::numeric_limits<TYPE>::max();
	if(left < 0 && right <0 )
	{
		return type_Max / (-(int)right) < (-(int)left);
	}else
	{
		return type_Max / right < left;
	}
}
template <typename TYPE>
bool mul_will_overflow_min(const TYPE left, const TYPE right)
{
	const TYPE& type_MIN = std::numeric_limits<TYPE>::min();
	if(right > 0)
	  return type_MIN /right > left;
	else
	  return type_MIN /left > right;
}
struct CONST* IC_MUL::constant_fold(struct CONST* fst,struct CONST* snd,bool *overflow)
{
	struct TYPE* type=usual_arithmetic_conversion(fst->type,snd->type);
	std::string common=type->op;
	bool overflow_=false;
	fst->cast_to(type);
	snd->cast_to(type);
	if(common=="int32")
	{
		if( ((fst->value.int32 >0 && snd->value.int32 >0) ||
			(fst->value.int32 < 0 && snd->value.int32 <0) ))
		{
			if(	mul_will_overflow_max<long int>(fst->value.int32,snd->value.int32))
				overflow_=true;
		}else 
		{
			if(mul_will_overflow_min<long int>(fst->value.int32,snd->value.int32))
				overflow_=true;
		}
		fst->value.int32 *=snd->value.int32;	
	}else if(common=="uint32")
	{
		if(mul_will_overflow_max<unsigned long int>(fst->value.uint32,snd->value.uint32))
			overflow_=true;
		fst->value.uint32 *= snd->value.uint32;	
	}else if(common=="float32")
	{
		fst->value.float32 *= snd->value.float32;			
	}else if(common=="float64")
	{
		fst->value.float64 *= snd->value.float64;					
	}
	if(overflow!=NULL)
	{
		*overflow=overflow_;
	}
	return fst;
}

struct CONST* IC_COMP::constant_fold(struct CONST* fst)
{
	struct TYPE* type=fst->type;
	type=integral_promotion(fst->type);
	fst->cast_to(type);
	std::string target=type->op;
	if(target=="int32")
	{
		fst->value.int32 =~fst->value.int32;	
	}else if(target=="uint32")
	{
		fst->value.uint32 =~fst->value.uint32;	
	}
	return fst;
}
struct CONST* IC_NEG::constant_fold(struct CONST* fst)
{
	struct TYPE* type=fst->type;
	if(fst->type->is_integral_type())
	{
		type=integral_promotion(fst->type);
		fst->cast_to(type);
	}
	std::string target=type->op;
	if(target=="int32")
	{
		fst->value.int32 =-fst->value.int32;	
	}else if(target=="uint32")
	{
		fst->value.uint32 =-(int)(fst->value.uint32);	
	}else if(target=="float32")
	{
		fst->value.float32 =-fst->value.float32;		
	}else if(target=="float64")
	{
		fst->value.float64 =-fst->value.float64;					
	}
	return fst;
}

struct CONST* IC_DIV::constant_fold(struct CONST* fst,struct CONST* snd)
{
	struct TYPE* type=usual_arithmetic_conversion(fst->type,snd->type);
	std::string common=type->op;
	fst->cast_to(type);
	snd->cast_to(type);
	if(snd->is_zero())
		return fst;
	if(common=="int32")
	{
		fst->value.int32 /=snd->value.int32;	
	}else if(common=="uint32")
	{
		fst->value.uint32 /= snd->value.uint32;	
	}else if(common=="float32")
	{
		fst->value.float32 /= snd->value.float32;			
	}else if(common=="float64")
	{
		fst->value.float64 /= snd->value.float64;					
	}
	return fst;
}
struct CONST* IC_MOD::constant_fold(struct CONST* fst,struct CONST* snd)
{
	struct TYPE* type=usual_arithmetic_conversion(fst->type,snd->type);
	std::string common=type->op;
	fst->cast_to(type);
	snd->cast_to(type);
	if(snd->is_zero())
		return fst;
	if(common=="int32")
	{
		fst->value.int32 %=snd->value.int32;	
	}else if(common=="uint32")
	{
		fst->value.uint32 %= snd->value.uint32;	
	}
	return fst;
}
struct CONST* IC_LSHIFT::constant_fold(struct CONST* fst,struct CONST* snd)
{
	struct TYPE* type=usual_arithmetic_conversion(fst->type,snd->type);
	std::string common=type->op;
	fst->cast_to(type);
	snd->cast_to(type);
	if(common=="int32")
	{
		fst->value.int32 <<=snd->value.int32;	
	}else if(common=="uint32")
	{
		fst->value.uint32 <<= snd->value.uint32;	
	}
	return fst;
}
struct CONST* IC_RSHIFT::constant_fold(struct CONST* fst,struct CONST* snd)
{
	struct TYPE* type=usual_arithmetic_conversion(fst->type,snd->type);
	std::string common=type->op;
	fst->cast_to(type);
	snd->cast_to(type);
	if(common=="int32")
	{
		fst->value.int32 >>=snd->value.int32;	
	}else if(common=="uint32")
	{
		fst->value.uint32 >>= snd->value.uint32;	
	}
	return fst;
}
struct CONST* IC_BITAND::constant_fold(struct CONST* fst,struct CONST* snd)
{
	struct TYPE* type=usual_arithmetic_conversion(fst->type,snd->type);
	std::string common=type->op;
	fst->cast_to(type);
	snd->cast_to(type);
	if(common=="int32")
	{
		fst->value.int32 &=snd->value.int32;	
	}else if(common=="uint32")
	{
		fst->value.uint32 &= snd->value.uint32;	
	}
	return fst;
}
struct CONST* IC_BITXOR::constant_fold(struct CONST* fst,struct CONST* snd)
{
	struct TYPE* type=usual_arithmetic_conversion(fst->type,snd->type);
	std::string common=type->op;
	fst->cast_to(type);
	snd->cast_to(type);
	if(common=="int32")
	{
		fst->value.int32 ^=snd->value.int32;	
	}else if(common=="uint32")
	{
		fst->value.uint32 ^= snd->value.uint32;	
	}
	return fst;
}
struct CONST* IC_BITOR::constant_fold(struct CONST* fst,struct CONST* snd)
{
	struct TYPE* type=usual_arithmetic_conversion(fst->type,snd->type);
	std::string common=type->op;
	fst->cast_to(type);
	snd->cast_to(type);
	if(common=="int32")
	{
		fst->value.int32 |=snd->value.int32;	
	}else if(common=="uint32")
	{
		fst->value.uint32 |= snd->value.uint32;	
	}
	return fst;
}
template<typename TYPE>
int relational_constant_fold(std::string op,TYPE member0,TYPE member1)
{
	if(op=="==")
	{
		return member0 == member1;
	}else if(op=="!=")
	{
		return member0 != member1;
	}else if(op=="<")
	{
		return member0 < member1;
	}else if(op==">=")
	{
		return member0 >= member1;
	}else if(op==">")
	{
		return member0 > member1;
	}else if(op=="<=")
	{
		return member0 <= member1;
	}else
		assert(0);
	return 0;
}
struct CONST* ISET::constant_fold(std::string op,struct CONST* fst,struct CONST* snd)
{
	struct TYPE* type=usual_arithmetic_conversion(fst->type,snd->type);
	std::string common=type->op;
	fst->cast_to(type);
	snd->cast_to(type);
	int result_value;
	if(common=="int32")
	{
		result_value=relational_constant_fold<long int>(op,fst->value.int32,snd->value.int32);
	}else if(common=="uint32")
	{
		result_value=relational_constant_fold<unsigned long int>(op,fst->value.uint32,snd->value.uint32);
	}else if(common=="float32")
	{
		result_value=relational_constant_fold<float>(op,fst->value.float32,snd->value.float32);			
	}else if(common=="float64")
	{
		result_value=relational_constant_fold<double>(op,fst->value.float64,snd->value.float64);				
	}
	struct CONST* result=new CONST();
	result->value.int32=result_value;
	result->type=TYPE::basic_type("int32");
	return result;
}



