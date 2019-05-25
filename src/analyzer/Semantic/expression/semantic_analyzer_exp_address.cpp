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
void exp_ADDR::check()
{
	this->check_operands();
	struct ADDRESS* operand0=this->operand_list[0]->addr;

	if( (operand0->is_lvalue && !operand0->is_BIT_FIELD())  ||
		operand0->type->is_function())
	{
		this->addr=MEM::addr_of(operand0);
		this->addr->type=TYPE::pointer_type(operand0->type);
		this->addr->is_lvalue=false;
	}else
	{
		::SCC_ERROR(pos,"& : illegal operand.");
		this->as_default();
	}
}
void exp_DEREF::check()
{
	this->check_operands();
	struct ADDRESS* operand0=this->operand_list[0]->addr;

	if(operand0->type->is_pointer())
	{
		struct POINTER_TYPE* ptr=static_cast<struct POINTER_TYPE*>(operand0->type);
		this->addr=MEM::deref_of(operand0);
		this->addr->type=ptr->type_;
		this->addr->is_lvalue=ptr->type_->is_object_type();
	}else
	{
		::SCC_ERROR(pos,"* : operand must be pointer.");
		this->as_default();
	}

}
void exp_INDEX::check()
{
	this->check_operands();
	struct ADDRESS* operand0=this->operand_list[0]->addr;
	struct ADDRESS* operand1=this->operand_list[1]->addr;

	if(!operand0->type->is_pointer())
	{
		std::swap(operand0,operand1);
	}
	if(operand0->type->is_pointer_to_object_type() && operand1->type->is_integral_type() )
	{
		struct ADDRESS* address;
		struct POINTER_TYPE* ptr=static_cast<struct POINTER_TYPE*>(operand0->type);
		bool overflow=false;
		struct CONST *scale=new CONST(ptr->type_->size);
		struct ADDRESS* offset=IC_MUL::new_(operand1,scale,&overflow);
		if(overflow)
		{
			::SCC_WARNNING(this->pos,"* : integral constant overflow.");
		}
		overflow=false;
		operand0->type=TYPE::basic_type("uint32");//change to uint32
		address=IC_ADD::new_(operand0,offset,&overflow);
		if(overflow)
		{
			::SCC_WARNNING(this->pos,"+ : integral constant overflow.");
		}
		address->type=ptr;
		this->addr=MEM::deref_of(address);
		this->addr->type=ptr->type_;
		this->addr->is_lvalue=ptr->type_->is_object_type();
	}else
	{
		::SCC_ERROR(pos,"[]: need array/pointer and integral type.");
		this->as_default();
	}
}
void exp_OBJ_MBSL::check()
{
	this->check_operands();
	struct ADDRESS* operand0=this->operand_list[0]->addr;

	if(operand0->type->is_struct_or_union())
	{
		struct iMEMBER* member;
		bool exist;
		if(operand0->type->is_struct())
		{
			struct STRUCT_TYPE *casted=static_cast<struct STRUCT_TYPE *>(operand0->type->unqualify());
			exist=casted->find_member(this->member,&member);
		}else
		{
			struct UNION_TYPE *casted=static_cast<struct UNION_TYPE *>(operand0->type->unqualify());
			exist=casted->find_member(this->member,&member);
		}
		if(exist)
		{

			struct ADDRESS* addr_of_operand0=MEM::addr_of(operand0);
			struct ADDRESS* address;
			if(operand0->type->is_struct())
			{
				struct CONST *offset=new CONST(member->offset);
				addr_of_operand0->type=TYPE::basic_type("uint32");//change to uint32
				address=IC_ADD::new_(addr_of_operand0,offset,NULL);
			}else
				address=addr_of_operand0;
			if(member->is_bit_field)
			{
				this->addr=new BIT_FIELD(member->type,address,member->bit_offset,member->width);
			}else
			{
				this->addr=MEM::deref_of(address);
			}
			this->addr->is_lvalue=operand0->is_lvalue;
			this->addr->type=member->type;
		}else
		{
			::SCC_ERROR(pos,"%s : named member not exist",this->member.c_str());
			this->as_default();
		}
	}else
	{
		::SCC_ERROR(pos,". : left operand is not a struct/union.");
		this->as_default();
	}
}
void exp_PTR_MBSL::check()
{
	this->check_operands();
	struct ADDRESS* operand0=this->operand_list[0]->addr;

	if(operand0->type->is_pointer())
	{
		struct POINTER_TYPE* ptr=static_cast<struct POINTER_TYPE *>(operand0->type);
		if(ptr->type_->is_struct_or_union())
		{
			struct iMEMBER* member;
			bool exist;
			if(ptr->type_->is_struct())
			{
				struct STRUCT_TYPE *casted=static_cast<struct STRUCT_TYPE *>(ptr->type_->unqualify());
				exist=casted->find_member(this->member,&member);
			}else
			{
				struct UNION_TYPE *casted=static_cast<struct UNION_TYPE *>(ptr->type_->unqualify());
				exist=casted->find_member(this->member,&member);
			}
			if(exist)
			{
				struct ADDRESS* address;
				if(ptr->type_->is_struct())
				{
					struct CONST *offset=new CONST(member->offset);
					operand0->type=TYPE::basic_type("uint32");//change to uint32
					address=IC_ADD::new_(operand0,offset,NULL);
				}else
					address=operand0;
				if(member->is_bit_field)
				{
					this->addr=new BIT_FIELD(member->type,address,member->bit_offset,member->width);
				}else
				{
					this->addr=MEM::deref_of(address);
				}
				this->addr->is_lvalue=true;
				this->addr->type=member->type;
			}else
			{
				::SCC_ERROR(pos,"%s : named member not exist",this->member.c_str());
				this->as_default();
			}
		}else
		{
			::SCC_ERROR(pos,"-> : left operand is not a pointer to a struct/union.");
			this->as_default();
		}
	}else
	{
		::SCC_ERROR(pos,"-> : left operand is not a pointer.");
		this->as_default();
	}
}

void exp_ID::check()
{
	struct IDENTIFIER *identifier=SCOPE::current->find(this->lexical_value);
	if(identifier!=NULL)
	{
		identifier->inc_ref();
		if(identifier->kind=="VARIABLE" && static_cast<struct iVARIABLE*>(identifier)->is_enum_constant)
		{
			struct CONST *c=new CONST();
			c->value.int32=static_cast<struct iVARIABLE*>(identifier)->enum_constant_value;
			c->type=TYPE::basic_type("int32");
			this->addr=c;
		}else
		{
			struct VAR* var=new VAR();
			var->var=identifier;
			var->type=TYPE::pointer_type(identifier->type);
			var->is_lvalue=false;
			this->addr=new MEM(var);
			this->addr->type=identifier->type;
			this->addr->is_lvalue=identifier->type->is_object_type();
		}
	}else
	{
		::SCC_ERROR(pos,"%s : undefined.",lexical_value.c_str());
		this->as_default();
	}
}
void exp_CAST::check()
{
	struct TYPE* to_type=this->type_name->get_type();

	this->expr->check();
	this->expr->adjust(this->op,false);
	struct ADDRESS* becast=this->expr->addr;

	if(!to_type->is_void())
	{
		if(becast->type->is_scalar_type() && to_type->is_scalar_type())
		{
			if( (to_type->is_pointer() && becast->type->is_floating_type()) ||
				(becast->type->is_pointer() && to_type->is_floating_type()) )
			{
				::SCC_ERROR(pos,"cannot convert between floating and pointer.");
				this->as_default();
			}else
				this->addr=IC_CVT::cast_to(becast,to_type);
		}else
		{
			::SCC_ERROR(pos,"need arithmetic or pointer type.");
			this->as_default();
		}
	}else//if(!this->type->is_void())
	{
		this->addr=becast;
		this->addr->type=TYPE::void_type();
	}
}

void exp_CONSTANT_INTEGER::check()
{
	std::string type_code="int32";
	union CONST_VALUE value;

	errno=0;
	value.int32=strtol(lexical_value.c_str(),NULL,0);
	if (errno == ERANGE && value.int32==LONG_MAX)
	{
		type_code="uint32";
		errno=0;
		value.uint32=strtoul(lexical_value.c_str(),NULL,0);	
		if (errno == ERANGE && value.uint32==ULONG_MAX)
		{
			::SCC_ERROR(pos,"constant too big.");
		}
	}
	if((lexical_value.back()=='L'|| lexical_value.back()=='l')&&lexical_value.size()>=2 &&
		(lexical_value[lexical_value.size()-2]=='U'|| lexical_value[lexical_value.size()-2]=='u'))
	{
		type_code="uint32";
		value.uint32=value.int32;
	}

	struct CONST *c=new CONST();
	c->value=value;
	c->type=TYPE::basic_type(type_code);
	this->addr=c;
}
void exp_CONSTANT_CHARACTER::check()
{
	//remove single quote."'a'" to "a";
	std::string str(this->lexical_value.begin()+1,this->lexical_value.end()-1);
	char value='\0';
	if(str[0]!='\\')
	{
		value=str[0];
		if(str.size()>1)
			::SCC_WARNNING(pos,"too long for character.");
	}else
	{
		if(str.size()==2)
		{
			char escape_char=str[1];
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
					::SCC_WARNNING(pos,"%c : unknown escape sequence.",escape_char);
			}//switch
		}else
		{
			if(str[1]=='0')
			{//'\0ooo'
				std::string octal(str.begin()+1,str.end());
				int i=strtol(octal.c_str(),NULL,0);
				if( i > UCHAR_MAX )
					::SCC_WARNNING(pos,"too big for character.");
				value=static_cast<char>(i);
			}else if(str[1]=='x' || str[1]=='X')
			{//'\xhh'
				std::string hex(str.begin(),str.end());
				hex[0]='0';
				int i=strtol(hex.c_str(),NULL,0);
				if( i > UCHAR_MAX )
					::SCC_WARNNING(pos,"too big for character.");
				value=static_cast<char>(i);
			}else
			{
				::SCC_WARNNING(pos,"%c : unknown escape sequence.",str[1]);
			}
		}
	}
	struct CONST *c=new CONST();
	c->value.int8=value;
	c->type=TYPE::basic_type("int8");
	this->addr=c;
}
void exp_CONSTANT_FLOATING::check()
{
	std::string type_code="float64";
	union CONST_VALUE value;

	errno=0;
	value.float64=strtod(lexical_value.c_str(),NULL);
	if (errno == ERANGE && value.float64==HUGE_VAL)
	{
		::SCC_ERROR(pos,"floating constant too big.");
	}
	if(lexical_value.back()=='f' || lexical_value.back()=='F')
	{
		type_code="float32";
		value.float32=static_cast<float>(value.float64);
	}

	struct CONST *c=new CONST();
	c->value=value;
	c->type=TYPE::basic_type(type_code);
	this->addr=c;
}
void exp_STRING::check()
{
	//remove single quote.""abc"" to "abc";
	std::string str(this->lexical_value.begin()+1,this->lexical_value.end()-1);
	str.append(1,'\0');
	this->addr=MEM::global_string(str);
}
