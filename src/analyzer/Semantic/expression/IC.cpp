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

#include "..\..\lib\common.h"
#include "..\semantic_analyzer.h"
#include "..\..\Syntax_Tree\syntax_tree.h"
#include "..\..\Syntax_Tree\syntax_tree_decl.h"
#include "..\..\Syntax_Tree\syntax_tree_stmt.h"
#include "..\..\Syntax_Tree\syntax_tree_exp.h"
#include "..\Symbol_Table\type.h"
#include "..\Symbol_Table\identifier.h"
#include "..\Symbol_Table\scope.h"
#include "IC.h"

void ADDRESS::inc_ref()
{
	this->ref++;
}
void ADDRESS::dec_ref()
{
	this->ref--;
}
void MEM::inc_ref()
{
	this->ref++;
	this->addr->inc_ref();
}
void MEM::dec_ref()
{
	this->ref--;
	this->addr->dec_ref();
}
void BIT_FIELD::inc_ref()
{
	this->ref++;
	this->addr->inc_ref();
}
void BIT_FIELD::dec_ref()
{
	this->ref--;
	this->addr->dec_ref();
}

std::string CONST::toString()
{
	std::string text;
	if(this->type->is_pointer())
	{
		text=string_format("%u",value.uint32); 	
	}else
	{
		std::string type_code=this->type->op;
		if(type_code=="int8")
		{
			text=string_format("%d",value.int8);
		}else if(type_code=="uint8")
		{
			text=string_format("%d",value.uint8);
		}else if(type_code=="int16")
		{
			text=string_format("%d",value.int16);
		}else if(type_code=="uint16")
		{
			text=string_format("%d",value.uint16);
		}else if(type_code=="int32")
		{
			text=string_format("%d",value.int32); 					
		}else if(type_code=="uint32")
		{
			text=string_format("%u",value.uint32); 
		}else if(type_code=="float32")
		{
			text=string_format("%f",value.float32); 			
		}else if(type_code=="float64")
		{
			text=string_format("%f",value.float64);  			
		}
	}
	return text;
}
std::string VAR::toString()
{
	return var->name;
}
std::string MEM::toString()
{
	return string_format("[%s]",addr->toString().c_str());
}
std::string BIT_FIELD::toString()
{
	return string_format("[%s]",addr->toString().c_str());
}
std::string TEMP::toString()
{
	return string_format("t_%d",id_); 	
}

void IC_INS::inc_ref()
{
	if(this->result!=NULL)
		this->result->inc_ref();
	if(this->operand0!=NULL)
		this->operand0->inc_ref();
	if(this->operand1!=NULL)
		this->operand1->inc_ref();
}
void IC_INS::dec_ref()
{
	if(this->result!=NULL)
		this->result->dec_ref();
	if(this->operand0!=NULL)
		this->operand0->dec_ref();
	if(this->operand1!=NULL)
		this->operand1->dec_ref();
}
void IC_INS::emit()
{
	this->inc_ref();
	if(this->label!=NULL)
		this->label->reference(&this->label);
	if(iFUNCTION::current_definition!=NULL)
		iFUNCTION::current_definition->insert_ins(this);
}
void IC_INS_BLOCK::start()
{
	static int id=0;
	if(name.empty())
	{
		name=string_format("L_%d",id++);
	}
	iFUNCTION::current_definition->insert_block(this);
}

void IC_INS_BLOCK::output_IC(std::string *buffer)
{
	buffer->append(string_format("%s:\n",name.c_str()));
	for(size_t i=0;i<this->ins_list.size();i++)
	{
		buffer->append(string_format("%s:\n",ins_list.at(i)->toString().c_str()));
	}
}
void IC_INS_BLOCK::show()
{
	::SCC_MSG("%s:",name.c_str());
	for(size_t i=0;i<this->ins_list.size();i++)
	{
		::SCC_MSG("%s",ins_list.at(i)->toString().c_str());
	}
}
void IC_INS_BLOCK::reference(struct IC_INS_BLOCK** r)
{
	if(r!=NULL)
	{
		this->ref++;
		this->reference_list.push_back(r);
	}
}
std::string IC_INS_BLOCK::asm_name()
{
	return string_format("%s@%s",this->name.c_str(),iFUNCTION::current_definition->name.c_str());
}
struct ADDRESS* MEM::deref_of(struct ADDRESS* operand0)
{
	if(operand0->is_MEM())
	{
		struct TEMP* result=new TEMP();
		result->type=operand0->type;
		return new MEM(IC_MOVE::new_(result,operand0));
	}
	else
		return new MEM(operand0);
}
struct ADDRESS* MEM::addr_of(struct ADDRESS* operand0)
{
	assert(operand0->is_MEM());
	return static_cast<struct MEM*>(operand0)->addr;
}
struct MEM* MEM::local_temp(struct TYPE*type)
{
	static int id=0;
	std::string name="lt_";
	struct iVARIABLE* local=IDENTIFIER::variable(type,name.append(int_to_string(id++)),"none","automatic",NULL,"none");
	iFUNCTION::current_definition->insert_local(local);

	struct VAR* var=new VAR();
	var->var=local;
	var->type=TYPE::pointer_type(local->type);
	var->is_lvalue=false;
	struct MEM* m=new MEM(var);
	m->type=local->type;
	m->is_lvalue=local->type->is_object_type();
	return m;
}
struct MEM* MEM::global_temp(struct TYPE*type,struct INITIALIZER_DATA* initializer)
{
	static int id=0;
	std::string name="gt_";
	name.append(int_to_string(id++));
	struct iVARIABLE* temp=IDENTIFIER::variable(type,name,"internal","static",initializer,"definition");
	SCOPE::file->insert(name,temp);

	struct VAR* var=new VAR();
	var->var=temp;
	var->type=TYPE::pointer_type(temp->type);
	var->is_lvalue=false;
	struct MEM* m=new MEM(var);
	m->type=temp->type;
	m->is_lvalue=temp->type->is_object_type();
	return m;
}
struct MEM* MEM::global_string(std::string value)
{
	static int id=0;
	std::string name="gs_";
	name.append(int_to_string(id++));
	struct TYPE* type=TYPE::array_type(TYPE::basic_type("int8"),value.size());
	struct INITIALIZER_DATA* initializer=new struct INITIALIZER_DATA();
	initializer->copy_action_string(0,value);
	struct iVARIABLE* gs=IDENTIFIER::variable(type,name,"internal","static",initializer,"definition");
	SCOPE::file->insert(name,gs);

	struct VAR* var=new VAR();
	var->var=gs;
	var->type=TYPE::pointer_type(gs->type);
	var->is_lvalue=false;
	struct MEM* m=new MEM(var);
	m->type=gs->type;
	m->is_lvalue=gs->type->is_object_type();
	return m;
}
//unary
struct ADDRESS* IC_COMP::new_(struct ADDRESS* operand0)
{
	if(operand0->is_CONST())
	{
		return IC_COMP::constant_fold(static_cast<struct CONST*>(operand0));
	}
	struct TYPE* type=operand0->type;
	type=integral_promotion(operand0->type);
	operand0=IC_CVT::cast_to(operand0,type);
	struct ADDRESS* a=unary_ins<IC_COMP>(operand0);
	a->type=type;
	return a;
}
struct ADDRESS* IC_NEG::new_(struct ADDRESS* operand0)
{
	if(operand0->is_CONST())
	{
		return IC_NEG::constant_fold(static_cast<struct CONST*>(operand0));
	}
	struct TYPE* type=operand0->type;
	if(operand0->type->is_integral_type())
	{
		type=integral_promotion(operand0->type);
		operand0=IC_CVT::cast_to(operand0,type);
	}
	struct ADDRESS* a=unary_ins<IC_NEG>(operand0);
	a->type=type;
	return a;
}
//add,sub,mul,div
struct ADDRESS* IC_ADD::new_(struct ADDRESS* operand0,struct ADDRESS* operand1,bool *overflow)
{
	struct TYPE* old_op0=NULL;
	struct TYPE* old_op1=NULL;
	if(operand0->type->is_pointer())
	{
		old_op0=operand0->type;
		operand0=IC_CVT::cast_to(operand0,TYPE::basic_type("uint32"));
	}
	if(operand1->type->is_pointer())
	{
		old_op0=operand1->type;
		operand1=IC_CVT::cast_to(operand1,TYPE::basic_type("uint32"));
	}
	struct TYPE* type=usual_arithmetic_conversion(operand0->type,operand1->type);
	operand0=IC_CVT::cast_to(operand0,type);
	operand1=IC_CVT::cast_to(operand1,type);
	if(operand0->is_CONST() && operand1->is_CONST())
	{
		return IC_ADD::constant_fold(static_cast<struct CONST*>(operand0),
			static_cast<struct CONST*>(operand1),overflow);
	}
	struct ADDRESS* a=binary_ins<IC_ADD>(operand0,operand1);
	a->type=type;
	if(old_op0!=NULL)
	{
		operand0->type=old_op0;
	}
	if(old_op1!=NULL)
	{
		operand1->type=old_op1;
	}
	return a;
}
struct ADDRESS* IC_SUB::new_(struct ADDRESS* operand0,struct ADDRESS* operand1,bool *overflow)
{
	struct TYPE* old_op0=NULL;
	struct TYPE* old_op1=NULL;
	if(operand0->type->is_pointer())
	{
		old_op0=operand0->type;
		operand0=IC_CVT::cast_to(operand0,TYPE::basic_type("uint32"));
	}
	if(operand1->type->is_pointer())
	{
		old_op0=operand1->type;
		operand1=IC_CVT::cast_to(operand1,TYPE::basic_type("uint32"));
	}
	struct TYPE* type=usual_arithmetic_conversion(operand0->type,operand1->type);
	operand0=IC_CVT::cast_to(operand0,type);
	operand1=IC_CVT::cast_to(operand1,type);
	if(operand0->is_CONST() && operand1->is_CONST())
	{
		return IC_SUB::constant_fold(static_cast<struct CONST*>(operand0),
			static_cast<struct CONST*>(operand1),overflow);
	}
	struct ADDRESS* a=binary_ins<IC_SUB>(operand0,operand1);
	a->type=type;
	if(old_op0!=NULL)
	{
		operand0->type=old_op0;
	}
	if(old_op1!=NULL)
	{
		operand1->type=old_op1;
	}
	return a;
}
struct ADDRESS* IC_MUL::new_(struct ADDRESS* operand0,struct ADDRESS* operand1,bool *overflow)
{
	if(operand0->is_CONST() && operand1->is_CONST())
	{
		return IC_MUL::constant_fold(static_cast<struct CONST*>(operand0),
			static_cast<struct CONST*>(operand1),overflow);
	}

	struct TYPE* type=usual_arithmetic_conversion(operand0->type,operand1->type);
	operand0=IC_CVT::cast_to(operand0,type);
	operand1=IC_CVT::cast_to(operand1,type);
	if(operand0->is_CONST() && static_cast<struct CONST*>(operand0)->is_one())
		return operand1;
	if(operand1->is_CONST() && static_cast<struct CONST*>(operand1)->is_one())
		return operand0;
	struct ADDRESS* a=binary_ins<IC_MUL>(operand0,operand1);
	a->type=type;
	return a;
}
struct ADDRESS* IC_DIV::new_(struct ADDRESS* operand0,struct ADDRESS* operand1)
{
	if(operand0->is_CONST() && operand1->is_CONST())
	{
		return IC_DIV::constant_fold(static_cast<struct CONST*>(operand0),
			static_cast<struct CONST*>(operand1));
	}
	struct TYPE* type=usual_arithmetic_conversion(operand0->type,operand1->type);
	operand0=IC_CVT::cast_to(operand0,type);
	operand1=IC_CVT::cast_to(operand1,type);
	if(operand1->is_CONST() && static_cast<struct CONST*>(operand1)->is_one())
		return operand0;
	struct ADDRESS* a=binary_ins<IC_DIV>(operand0,operand1);
	a->type=type;
	return a;
}
struct ADDRESS* IC_MOD::new_(struct ADDRESS* operand0,struct ADDRESS* operand1)
{
	if(operand0->is_CONST() && operand1->is_CONST())
	{
		return IC_MOD::constant_fold(static_cast<struct CONST*>(operand0),
			static_cast<struct CONST*>(operand1));
	}
	struct TYPE* type=usual_arithmetic_conversion(operand0->type,operand1->type);
	operand0=IC_CVT::cast_to(operand0,type);
	operand1=IC_CVT::cast_to(operand1,type);
	if(operand1->is_CONST() && static_cast<struct CONST*>(operand1)->is_one())
		return new CONST(0);
	struct ADDRESS* a=binary_ins<IC_MOD>(operand0,operand1);
	a->type=type;
	return a;
}
struct ADDRESS* IC_LSHIFT::new_(struct ADDRESS* operand0,struct ADDRESS* operand1)
{
	if(operand0->is_CONST() && operand1->is_CONST())
	{
		return IC_LSHIFT::constant_fold(static_cast<struct CONST*>(operand0),
			static_cast<struct CONST*>(operand1));
	}
	struct TYPE* type=usual_arithmetic_conversion(operand0->type,operand1->type);
	operand0=IC_CVT::cast_to(operand0,type);
	operand1=IC_CVT::cast_to(operand1,type);
	if(operand1->is_CONST() && static_cast<struct CONST*>(operand1)->is_zero())
		return operand0;
	struct ADDRESS* a=binary_ins<IC_LSHIFT>(operand0,operand1);
	a->type=type;
	return a;
}
struct ADDRESS* IC_RSHIFT::new_(struct ADDRESS* operand0,struct ADDRESS* operand1)
{
	if(operand0->is_CONST() && operand1->is_CONST())
	{
		return IC_RSHIFT::constant_fold(static_cast<struct CONST*>(operand0),
			static_cast<struct CONST*>(operand1));
	}
	struct TYPE* type=usual_arithmetic_conversion(operand0->type,operand1->type);
	operand0=IC_CVT::cast_to(operand0,type);
	operand1=IC_CVT::cast_to(operand1,type);
	if(operand1->is_CONST() && static_cast<struct CONST*>(operand1)->is_zero())
		return operand0;
	struct ADDRESS* a=binary_ins<IC_RSHIFT>(operand0,operand1);
	a->type=type;
	return a;
}
struct ADDRESS* IC_BITAND::new_(struct ADDRESS* operand0,struct ADDRESS* operand1)
{
	if(operand0->is_CONST() && operand1->is_CONST())
	{
		return IC_BITAND::constant_fold(static_cast<struct CONST*>(operand0),
			static_cast<struct CONST*>(operand1));
	}
	struct TYPE* type=usual_arithmetic_conversion(operand0->type,operand1->type);
	operand0=IC_CVT::cast_to(operand0,type);
	operand1=IC_CVT::cast_to(operand1,type);
	struct ADDRESS* a=binary_ins<IC_BITAND>(operand0,operand1);
	a->type=type;
	return a;
}
struct ADDRESS* IC_BITXOR::new_(struct ADDRESS* operand0,struct ADDRESS* operand1)
{
	if(operand0->is_CONST() && operand1->is_CONST())
	{
		return IC_BITXOR::constant_fold(static_cast<struct CONST*>(operand0),
			static_cast<struct CONST*>(operand1));
	}
	struct TYPE* type=usual_arithmetic_conversion(operand0->type,operand1->type);
	operand0=IC_CVT::cast_to(operand0,type);
	operand1=IC_CVT::cast_to(operand1,type);
	struct ADDRESS* a=binary_ins<IC_BITXOR>(operand0,operand1);
	a->type=type;
	return a;
}
struct ADDRESS* IC_BITOR::new_(struct ADDRESS* operand0,struct ADDRESS* operand1)
{
	if(operand0->is_CONST() && operand1->is_CONST())
	{
		return IC_BITOR::constant_fold(static_cast<struct CONST*>(operand0),
			static_cast<struct CONST*>(operand1));
	}
	struct TYPE* type=usual_arithmetic_conversion(operand0->type,operand1->type);
	operand0=IC_CVT::cast_to(operand0,type);
	operand1=IC_CVT::cast_to(operand1,type);
	struct ADDRESS* a=binary_ins<IC_BITOR>(operand0,operand1);
	a->type=type;
	return a;
}
//convert
struct ADDRESS* IC_CVT::cast_to(struct ADDRESS* becast,struct TYPE* to_type)
{
	if(becast->type->is_compatible(to_type))
	{
		becast->type=to_type;
		return becast;
	}
	if(becast->is_CONST())
	{
		static_cast<struct CONST*>(becast)->cast_to(to_type);
		return becast;
	}
	if(becast->type->is_arithmetic_type() && to_type->is_arithmetic_type())
	{
		if(becast->type->is_integral_type() && to_type->is_integral_type() && becast->type->size==to_type->size)
		{
			becast->type=to_type;
			return becast;
		}
		return IC_CVT::new_(becast,to_type);
	}else if(becast->type->is_pointer() && to_type->is_integral_type())
	{
		if(to_type->unqualify()->op=="int32" || to_type->unqualify()->op=="uint32")
		{
			becast->type=to_type;
			return becast;
		}
		return IC_CVT::new_(becast,to_type); 
	}else if(becast->type->is_integral_type() && to_type->is_pointer())
	{
		if(becast->type->unqualify()->op=="int32" || becast->type->unqualify()->op=="uint32")
		{
			becast->type=to_type;
			return becast;
		}
		return IC_CVT::new_(becast,to_type); 
	}else if(becast->type->is_pointer() && to_type->is_pointer())
	{
		becast->type=to_type;
		return becast;
	}else
		assert(0);
	return NULL;
}
static std::string TYPE_CVT_CODE(struct TYPE* type)
{
	std::string op=type->op;
	std::string code;
	if(op=="int8")
		code="ib";
	if(op=="uint8")
		code="ub";
	if(op=="int16")
		code="iw";
	if(op=="uint16")
		code="uw";
	if(op=="int32")
		code="id";
	if(op=="uint32" || type->is_pointer())
		code="ud";
	if(op=="float32")
		code="fd";
	if(op=="float64")
		code="fq";
	return code;
}
struct ADDRESS* IC_CVT::new_(struct ADDRESS* becast,struct TYPE* to_type)
{
	IC_CVT* ins=new IC_CVT();
	ins->from=TYPE_CVT_CODE(becast->type->unqualify());
	ins->to=TYPE_CVT_CODE(to_type->unqualify());
	struct TEMP* t=new TEMP();
	t->ins=ins;
	ins->result=t;
	ins->operand0=becast;
	t->type=to_type;

	ins->emit();
	return t;
}
//move,movem,clear
struct ADDRESS* IC_MOVE::new_(struct ADDRESS* operand0,struct ADDRESS* operand1)
{
	IC_MOVE* ins=new IC_MOVE();
	ins->operand0=operand0;
	ins->operand1=operand1;

	ins->emit();
	return operand0;
}
struct ADDRESS* IC_MOVEM::new_(struct ADDRESS* operand0,struct ADDRESS* operand1,unsigned int size)
{
	IC_MOVEM* ins=new IC_MOVEM();
	ins->operand0=operand0;
	ins->operand1=operand1;
	ins->size=size;

	ins->emit();
	return operand0;
}
struct ADDRESS* IC_CLEAR::new_(struct ADDRESS* operand0,unsigned int size)
{
	IC_CLEAR* ins=new IC_CLEAR();
	ins->operand0=operand0;
	ins->size=size;

	ins->emit();
	return operand0;
}
//logic
void JUMP::new_(struct IC_INS_BLOCK* label)
{
	struct JUMP* ins=new JUMP();
	ins->label=label;

	ins->emit();
}
static std::string not_op(std::string op)
{
	if(op=="==")
		return "!=";
	if(op=="!=")
		return "==";
	if(op=="<")
		return ">=";
	if(op==">=")
		return "<";
	if(op==">")
		return "<=";
	if(op=="<=")
		return ">";

	assert(0);
	return op;
}
void IJUMP::new_(std::string op,ADDRESS* operand0,struct ADDRESS* operand1,struct IC_INS_BLOCK* iftrue,struct IC_INS_BLOCK* iffalse)
{
	struct IJUMP* ins=new IJUMP();
	ins->operand0=operand0;
	ins->operand1=operand1;
	if(iftrue!=NULL && iffalse!=NULL)
	{
		ins->op=op;
		ins->label=iftrue;
		JUMP::new_(iffalse);
	}else if(iftrue!=NULL)
	{
		ins->op=op;
		ins->label=iftrue;
	}else if(iffalse!=NULL)
	{
		ins->op=not_op(op);
		ins->label=iffalse;
	}
	ins->emit();
}
struct ADDRESS* ISET::new_(std::string op,struct ADDRESS* operand0,struct ADDRESS* operand1)
{
	if(operand0->is_CONST() && operand1->is_CONST())
	{
		return ISET::constant_fold(op,static_cast<struct CONST*>(operand0),
			static_cast<struct CONST*>(operand1));
	}
	struct TYPE* type=usual_arithmetic_conversion(operand0->type,operand1->type);
	operand0=IC_CVT::cast_to(operand0,type);
	operand1=IC_CVT::cast_to(operand1,type);
	struct ISET* ins=new ISET(op,operand0,operand1);
	return ins->result;
}

//helper handler
bool is_compatible_struct_or_union(struct TYPE* type0,struct TYPE* type1)
{
	if(type0->is_struct() && type1->is_struct() && type0->is_compatible(type1))
		return true;
	if(type0->is_union() && type1->is_union() && type0->is_compatible(type1))
		return true;
	return false;
}
struct BASIC_TYPE * integral_promotion(struct TYPE* fst)
{
	struct BASIC_TYPE *fst_casted=static_cast<struct BASIC_TYPE *>(fst);
	if(fst_casted->is_int8() || fst_casted->is_uint8() ||
		fst_casted->is_int16() || fst_casted->is_uint16())
		return TYPE::basic_type("int32");
	return fst_casted;
}
struct BASIC_TYPE * usual_arithmetic_conversion(struct TYPE* fst,struct TYPE* snd)
{
	struct BASIC_TYPE *fst_casted=static_cast<struct BASIC_TYPE *>(fst);
	struct BASIC_TYPE *snd_casted=static_cast<struct BASIC_TYPE *>(snd);
	std::string type_code;
	if(fst_casted->is_float64() || snd_casted->is_float64() )
	{
		type_code="float64";
	}else if(fst_casted->is_float32() || snd_casted->is_float32() )
	{
		type_code="float32";
	}else if(fst_casted->is_uint32() || snd_casted->is_uint32() )
	{
		type_code="uint32";
	}else
		type_code="int32";
	return TYPE::basic_type(type_code);
}