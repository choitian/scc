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
#include "..\semantic_analyzer.h"
#include "..\..\Syntax_Tree\syntax_tree.h"
#include "..\..\Syntax_Tree\syntax_tree_decl.h"
#include "..\..\Syntax_Tree\syntax_tree_stmt.h"
#include "..\..\Syntax_Tree\syntax_tree_exp.h"
#include "..\Symbol_Table\type.h"
#include "..\Symbol_Table\identifier.h"
#include "..\Symbol_Table\scope.h"
#include "..\constant\constant.h"

bool is_compatible_struct_or_union(struct TYPE* type0,struct TYPE* type1);
struct BASIC_TYPE * integral_promotion(struct TYPE* fst);
struct BASIC_TYPE * usual_arithmetic_conversion(struct TYPE* fst,struct TYPE* snd);
struct ADDRESS
{
	struct TYPE* type;
	bool is_lvalue;
	int ref;
	virtual bool is_CONST() {return false; }
	virtual bool is_VAR() {return false; }
	virtual bool is_MEM() {return false; }
	virtual bool is_BIT_FIELD() {return false; }
	virtual bool is_TEMP() {return false; }
	virtual void inc_ref();
	virtual void dec_ref();
	ADDRESS()
	{
		type=NULL;
		is_lvalue=false;
		ref=0;
	}
//debug
	virtual std::string toString()=0;
};
struct CONST:public ADDRESS
{
	union CONST_VALUE value;
	bool is_CONST() {return true; }
	void cast_to(struct TYPE* type);
	bool is_zero();
	bool is_one();
	int as_int32();
	unsigned int as_uint32();
	static struct CONST* constant_expression_2_const(struct exp* expr);
//translate
	std::string asm_value();
///////////////////
template<typename TYPE>
void cast_from(TYPE* member)
{
	std::string from=this->type->op;
	if(from=="int8")
	{
		*member=static_cast<TYPE>(this->value.int8);
	}else if(from=="uint8")
	{
		*member=static_cast<TYPE>(this->value.uint8);	
	}else if(from=="int16")
	{
		*member=static_cast<TYPE>(this->value.int16);				
	}else if(from=="uint16")
	{
		*member=static_cast<TYPE>(this->value.uint16);						
	}else if(from=="int32")
	{
		*member=static_cast<TYPE>(this->value.int32);				
	}else if(from=="uint32")
	{
		*member=static_cast<TYPE>(this->value.uint32);							
	}else if(from=="float32")
	{
		*member=static_cast<TYPE>(this->value.float32);						
	}else if(from=="float64")
	{
		*member=static_cast<TYPE>(this->value.float64);				
	}else if(from=="PTR")
		*member=static_cast<TYPE>(this->value.uint32);
	else
		assert(0);//never here.
}
	CONST(){}
	CONST(int value)
	{
		this->value.int32=value;
		this->type=TYPE::basic_type("int32");
	}
	CONST(unsigned int value)
	{
		this->value.uint32=value;
		this->type=TYPE::basic_type("uint32");
	}
//debug
	std::string toString();
};
struct VAR:public ADDRESS
{
	struct IDENTIFIER* var;
	unsigned int offset;
	bool is_VAR() {return true; }
	static struct VAR* new_(struct IDENTIFIER* var,struct TYPE* type,unsigned int offset)
	{
		struct VAR* v=new struct VAR();
		v->var=var;
		v->offset=offset;
		v->type=type;
		v->is_lvalue=false;
		return v;
	}
	VAR()
	{
		offset=0;
		var=NULL;
	}
//translate
	std::string asm_value_offset(unsigned int the_offset);
	std::string asm_value();
	void load_to_reg(struct REG* reg);
///////////////////
//debug
	std::string toString();
};
struct MEM:public ADDRESS
{
	struct ADDRESS* addr;
	bool is_MEM() {return true; }
	virtual void inc_ref();
	virtual void dec_ref();
	MEM(struct ADDRESS* addr)
	{
		this->addr=addr;
	}
	static struct ADDRESS* deref_of(struct ADDRESS* operand0);
	static struct ADDRESS* addr_of(struct ADDRESS* operand0);
	static struct MEM* local_temp(struct TYPE*type);
	static struct MEM* global_temp(struct TYPE*type,struct INITIALIZER_DATA* initializer);
	static struct MEM* global_string(std::string value);
//translate
	std::string var_asm_value(unsigned int offset);
	std::string asm_value(struct REG* option_reg=NULL);
///////////////////
//debug
	std::string toString();
};
struct BIT_FIELD:public ADDRESS
{
	struct ADDRESS* addr;
	bool is_unsigned;
	size_t bit_offset;
	size_t width;
	bool is_BIT_FIELD() {return true; }
	virtual void inc_ref();
	virtual void dec_ref();
	BIT_FIELD(struct TYPE* type,struct ADDRESS* addr,size_t bit_offset,size_t width)
	{
		this->type=type;
		this->is_unsigned=type->is_unsigned();
		this->addr=addr;
		this->bit_offset=bit_offset;
		this->width=width;
	}
//translate
	std::string asm_value(struct REG* option_reg=NULL);
	void store_from_reg(struct REG* reg);
	void load_to_reg(struct REG* reg);
///////////////////
//debug
	std::string toString();
};
struct TEMP:public ADDRESS
{
	struct IC_INS* ins;
	bool is_TEMP() {return true; }
//translate
	struct MEM* mem;
	struct REG* reg;
	void save_floating_st();
	void save(struct REG* r);
//////////
	TEMP()
	{
		static int id=0;
		id_=id++;
		reg=NULL;
		mem=NULL;
	}
//translate
//debug
	std::string toString();
private:
	int id_;
};

struct IC_INS
{
	std::string op;
	struct ADDRESS* operand0;
	struct ADDRESS* operand1;
	struct TEMP* result;
	struct IC_INS_BLOCK* label;
	void emit();
	virtual bool is_jump_ins(){return false;}
	virtual bool is_addition() {return false;}
	virtual bool is_subtraction() {return false;}
	IC_INS()
	{
		operand0=NULL;
		operand1=NULL;
		result=NULL;
		label=NULL;
	}
	void inc_ref();
	void dec_ref();
//translate
	virtual void pre_translate();
	virtual void translate()=0;
	virtual void post_translate();
//debug
	virtual std::string toString()=0;
};
struct IC_INS_BLOCK
{
	std::vector<struct IC_INS*> ins_list;
	void start();
	void insert(struct IC_INS* ins)
	{
		ins_list.push_back(ins);
	}
	static struct IC_INS_BLOCK* new_()
	{
		return new IC_INS_BLOCK();
	}
	static struct IC_INS_BLOCK* new_(std::string name)
	{
		struct IC_INS_BLOCK* iblock=new IC_INS_BLOCK();
		iblock->name=name;
		return iblock;
	}

	void show();
	void output_IC(std::string *buffer);
	std::string asm_name();
	std::string name;

	void reference(struct IC_INS_BLOCK** r);
	int ref;
	IC_INS_BLOCK()
	{
		ref=0;
	}
	std::vector<struct IC_INS_BLOCK** > reference_list;
	void translate();
};
template<typename INS_TYPE>
TEMP* unary_ins(struct ADDRESS* operand0)
{
	INS_TYPE* ins=new INS_TYPE();
	struct TEMP* t=new TEMP();
	t->ins=ins;
	ins->result=t;
	ins->operand0=operand0;
	ins->emit();
	return t;
}
template<typename INS_TYPE>
TEMP* binary_ins(struct ADDRESS* operand0,struct ADDRESS* operand1,std::string op=std::string())
{
	INS_TYPE* ins=new INS_TYPE();
	struct TEMP* t=new TEMP();
	t->ins=ins;
	ins->result=t;
	ins->operand0=operand0;
	ins->operand1=operand1;

	ins->emit();
	return t;
}
struct IC_COMP:public IC_INS
{
	IC_COMP()
	{
		op="~";
	}
	static struct CONST* constant_fold(struct CONST* fst);
	static struct ADDRESS* new_(struct ADDRESS* operand0);
//translate
	virtual void translate();
//debug
	std::string toString()
	{
		return string_format("%s = %s %s",result->toString().c_str(),op.c_str(),
			operand0->toString().c_str());
	}
};
struct IC_NEG:public IC_INS
{
	IC_NEG()
	{
		op="-";
	}
	static struct CONST* constant_fold(struct CONST* fst);
	static struct ADDRESS* new_(struct ADDRESS* operand0);
//translate
	virtual void translate();
//debug
	std::string toString()
	{
		return string_format("%s = %s %s",result->toString().c_str(),op.c_str(),
			operand0->toString().c_str());
	}
};
struct IC_ADD:public IC_INS
{
	static struct CONST* constant_fold(struct CONST* fst,struct CONST* snd,bool *overflow);
	static struct ADDRESS* new_(struct ADDRESS* operand0,struct ADDRESS* operand1,bool *overflow);
	IC_ADD()
	{
		op="+";
	}
	virtual bool is_addition() {return true;}
//translate
	virtual void translate();
//debug
	std::string toString()
	{
		return string_format("%s = %s %s %s",result->toString().c_str(),
			operand0->toString().c_str(),op.c_str(),operand1->toString().c_str());
	}
};
struct IC_SUB:public IC_INS
{
	static struct CONST* constant_fold(struct CONST* fst,struct CONST* snd,bool *overflow);
	static struct ADDRESS* new_(struct ADDRESS* operand0,struct ADDRESS* operand1,bool *overflow);
	IC_SUB()
	{
		op="-";
	}
	virtual bool is_subtraction() {return true;}
//translate
	virtual void translate();
//debug
	std::string toString()
	{
		return string_format("%s = %s %s %s",result->toString().c_str(),
			operand0->toString().c_str(),op.c_str(),operand1->toString().c_str());
	}
};
struct IC_MUL:public IC_INS
{
	static struct CONST* constant_fold(struct CONST* fst,struct CONST* snd,bool *overflow);
	static struct ADDRESS* new_(struct ADDRESS* operand0,struct ADDRESS* operand1,bool *overflow);
	IC_MUL()
	{
		op="*";
	}
//translate
	virtual void translate();
//debug
	std::string toString()
	{
		return string_format("%s = %s %s %s",result->toString().c_str(),
			operand0->toString().c_str(),op.c_str(),operand1->toString().c_str());
	}
};
struct IC_DIV:public IC_INS
{
	static struct CONST* constant_fold(struct CONST* fst,struct CONST* snd);
	static struct ADDRESS* new_(struct ADDRESS* operand0,struct ADDRESS* operand1);
	IC_DIV()
	{
		op="/";
	}
//translate
	virtual void translate();
//debug
	std::string toString()
	{
		return string_format("%s = %s %s %s",result->toString().c_str(),
			operand0->toString().c_str(),op.c_str(),operand1->toString().c_str());
	}
};
struct IC_MOD:public IC_INS
{
	static struct CONST* constant_fold(struct CONST* fst,struct CONST* snd);
	static struct ADDRESS* new_(struct ADDRESS* operand0,struct ADDRESS* operand1);
	IC_MOD()
	{
		op="%";
	}
//translate
	virtual void translate();
//debug
	std::string toString()
	{
		return string_format("%s = %s %s %s",result->toString().c_str(),
			operand0->toString().c_str(),op.c_str(),operand1->toString().c_str());
	}
};
struct IC_LSHIFT:public IC_INS
{
	static struct CONST* constant_fold(struct CONST* fst,struct CONST* snd);
	static struct ADDRESS* new_(struct ADDRESS* operand0,struct ADDRESS* operand1);
	IC_LSHIFT()
	{
		op="<<";
	}
//translate
	virtual void translate();
//debug
	std::string toString()
	{
		return string_format("%s = %s %s %s",result->toString().c_str(),
			operand0->toString().c_str(),op.c_str(),operand1->toString().c_str());
	}
};
struct IC_RSHIFT:public IC_INS
{
	static struct CONST* constant_fold(struct CONST* fst,struct CONST* snd);
	static struct ADDRESS* new_(struct ADDRESS* operand0,struct ADDRESS* operand1);
	IC_RSHIFT()
	{
		op=">>";
	}
//translate
	virtual void translate();
//debug
	std::string toString()
	{
		return string_format("%s = %s %s %s",result->toString().c_str(),
			operand0->toString().c_str(),op.c_str(),operand1->toString().c_str());
	}
};
struct IC_BITAND:public IC_INS
{
	static struct CONST* constant_fold(struct CONST* fst,struct CONST* snd);
	static struct ADDRESS* new_(struct ADDRESS* operand0,struct ADDRESS* operand1);
	IC_BITAND()
	{
		op="&";
	}
//translate
	virtual void translate();
//debug
	std::string toString()
	{
		return string_format("%s = %s %s %s",result->toString().c_str(),
			operand0->toString().c_str(),op.c_str(),operand1->toString().c_str());
	}
};
struct IC_BITXOR:public IC_INS
{
	static struct CONST* constant_fold(struct CONST* fst,struct CONST* snd);
	static struct ADDRESS* new_(struct ADDRESS* operand0,struct ADDRESS* operand1);
	IC_BITXOR()
	{
		op="^";
	}
//translate
	virtual void translate();
//debug
	std::string toString()
	{
		return string_format("%s = %s %s %s",result->toString().c_str(),
			operand0->toString().c_str(),op.c_str(),operand1->toString().c_str());
	}
};
struct IC_BITOR:public IC_INS
{
	static struct CONST* constant_fold(struct CONST* fst,struct CONST* snd);
	static struct ADDRESS* new_(struct ADDRESS* operand0,struct ADDRESS* operand1);
	IC_BITOR()
	{
		op="|";
	}
//translate
	virtual void translate();
//debug
	std::string toString()
	{
		return string_format("%s = %s %s %s",result->toString().c_str(),
			operand0->toString().c_str(),op.c_str(),operand1->toString().c_str());
	}
};
struct IC_CVT:public IC_INS
{
	static struct ADDRESS* cast_to(struct ADDRESS* becast,struct TYPE* to_type);
	static struct ADDRESS* new_(struct ADDRESS* becast,struct TYPE* to_type);
	std::string from;
	std::string to;
	IC_CVT()
	{
		op="CVT";
	}
//translate
	virtual void translate();
//debug
	std::string toString()
	{
		std::string cvt_op=string_format("%s2%s",from.c_str(),to.c_str());
		return string_format("%s = %s %s",result->toString().c_str(),cvt_op.c_str(),operand0->toString().c_str());
	}
};

struct IC_MOVE:public IC_INS
{
	static struct ADDRESS* new_(struct ADDRESS* operand0,struct ADDRESS* operand1);
//translate
	virtual void translate();
//debug
	std::string toString()
	{
		return string_format("%s = %s",operand0->toString().c_str(),operand1->toString().c_str());
	}
};
struct IC_MOVEM:public IC_INS
{
	unsigned int size;
	static struct ADDRESS* new_(struct ADDRESS* operand0,struct ADDRESS* operand1,unsigned int size);
//translate
	virtual void translate();
//debug
	std::string toString()
	{
		return string_format("MOVEM %s,%s,%d",operand0->toString().c_str(),operand1->toString().c_str(),size);
	}
};
struct IC_CLEAR:public IC_INS
{
	unsigned int size;
	static struct ADDRESS* new_(struct ADDRESS* operand0,unsigned int size);
//translate
	virtual void translate();
//debug
	std::string toString()
	{
		return string_format("CLEAR %s,%d",operand0->toString().c_str(),size);
	}
};

struct JUMP:public IC_INS
{
	virtual bool is_jump_ins(){return true;}
//translate
	virtual void translate();
//debug
	std::string toString()
	{
		return string_format("goto %s",label->name.c_str());
	}
	JUMP()
	{
		op="JUMP";
	}
	static void new_(struct IC_INS_BLOCK* label);
};
struct IJUMP:public IC_INS
{
	virtual bool is_jump_ins(){return true;}
//translate
	virtual void translate();
//debug
	std::string toString()
	{
		return string_format("if %s %s %s goto %s",operand0->toString().c_str(),op.c_str(),operand1->toString().c_str(),
			label->name.c_str());
	}
static void new_(std::string op,struct ADDRESS* operand0,struct ADDRESS* operand1,struct IC_INS_BLOCK* iftrue,struct IC_INS_BLOCK* iffalse);
};
struct ISET:public IC_INS
{
	ISET(std::string op,struct ADDRESS* operand0,struct ADDRESS* operand1)
	{
		struct TEMP* t=new TEMP();
		t->ins=this;
		t->type=TYPE::basic_type("int32");

		this->result=t;
		this->operand0=operand0;
		this->operand1=operand1;
		this->op=op;

		this->emit();
	}
	static struct CONST* constant_fold(std::string op,struct CONST* fst,struct CONST* snd);
	static struct ADDRESS* new_(std::string op,struct ADDRESS* operand0,struct ADDRESS* operand1);
//translate
	virtual void translate();
//debug
	std::string toString()
	{
		return string_format("if %s %s %s set %s",operand0->toString().c_str(),op.c_str(),operand1->toString().c_str(),
			result->toString().c_str());
	}
static void new_(std::string op,std::string not_op,ADDRESS* operand0,struct ADDRESS* operand1,struct IC_INS_BLOCK* iftrue,struct IC_INS_BLOCK* iffalse);
};

struct RET:public IC_INS
{
	virtual bool is_jump_ins(){return true;}
	RET()
	{
		op="ret";
	}
	static void new_(struct ADDRESS* operand0)
	{
		struct RET* ins=new RET();
		ins->operand0=operand0;
		ins->emit();
	}
//translate
	virtual void translate();
//debug
	std::string toString()
	{
		return string_format("%s %s",op.c_str(),operand0->toString().c_str());
	}
};
struct PARAM:public IC_INS
{
	PARAM()
	{
		op="param";
	}
	static void new_(struct ADDRESS* operand0)
	{
		struct PARAM* ins=new PARAM();
		ins->operand0=operand0;

		ins->emit();
	}
//translate
	virtual void translate();
//debug
	std::string toString()
	{
		return string_format("%s %s",op.c_str(),operand0->toString().c_str());
	}
};
struct CALL:public IC_INS
{
	unsigned int stack_size;
	virtual bool is_jump_ins(){return true;}
	CALL()
	{
		op="call";
	}
	static struct ADDRESS* new_(struct TYPE* type,struct ADDRESS* operand0,unsigned int stack_size)
	{
		CALL* ins=new CALL();
		struct TEMP* t=new TEMP();
		t->ins=ins;
		ins->result=t;
		ins->operand0=operand0;
		ins->stack_size=stack_size;
		t->type=type;

		ins->emit();
		return t;
	}
//translate
	virtual void translate();
//debug
	std::string toString()
	{
		return string_format("%s = %s %s,%u",result->toString().c_str(),op.c_str(),operand0->toString().c_str(),stack_size);
	}
};

struct NOP:public IC_INS
{
	NOP()
	{
		op="NOP";
	}
	static void new_()
	{
		struct NOP* ins=new NOP();
		ins->emit();
	}
	virtual void translate(){}
//debug
	std::string toString()
	{
		return "NOP";
	}
};



