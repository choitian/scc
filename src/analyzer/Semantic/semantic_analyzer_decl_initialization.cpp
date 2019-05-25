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

#include "..\lib\common.h"
#include "semantic_analyzer.h"
#include "..\Syntax_Tree\syntax_tree.h"
#include "..\Syntax_Tree\syntax_tree_decl.h"
#include "..\Syntax_Tree\syntax_tree_stmt.h"
#include "Symbol_Table\type.h"
#include "Symbol_Table\identifier.h"
#include "Symbol_Table\scope.h"
#include "expression\IC.h"

static void last_bit_field_unit_initialization(struct INITIALIZER_DATA* ini_data);
static void padding(unsigned int offset,struct INITIALIZER_DATA* ini_data);
static void compound_check_initializer(struct TYPE* type,
	std::vector<struct syntax_node*>::iterator *cur,
	std::vector<struct syntax_node*>::iterator *end,bool standalone,
	unsigned int offset,struct INITIALIZER_DATA* ini_data);
static void direct_check_initializer(struct TYPE* type,struct INITIALIZER* ini,
	unsigned int offset,struct INITIALIZER_DATA* ini_data,bool is_bit_field,struct iMEMBER* bit_member);
void INITIALIZER::check()
{
	if(!this->value_checked)
	{
		struct exp* e=static_cast<struct exp*>(this->value);
		e->check();
		e->adjust(this->op,false);
		this->v_addr=e->addr;
	}
}


bool is_aggregate_type(struct TYPE* type)
{
	return type->is_struct() ||type->is_array();
}
bool is_compound(struct INITIALIZER* ini)
{
	return ini->value==NULL;
}
static struct TYPE* member_of_index(struct TYPE* type,unsigned int index,unsigned int *member_offset,bool *is_bit_field,struct iMEMBER **bit_member)
{
	struct TYPE* member_type=NULL;
	if(is_bit_field!=NULL)
		*is_bit_field=false;
	if(bit_member!=NULL)
		*bit_member=NULL;
	if(type->is_array())
	{
		struct ARRAY_TYPE* a=static_cast<struct ARRAY_TYPE*>(type);
		if(type->is_incomplete())
		{
			a->number_of_element_=index;
			if(member_offset!=NULL)
				*member_offset=index*a->type_->size;
			member_type= a->type_;
		}
		else
		{
			if(index < a->number_of_element_)
			{
				if(member_offset!=NULL)
					*member_offset=index*a->type_->size;
				member_type= a->type_;
			}
		}
	}
	else 
	{
		struct iMEMBER *member=NULL;
		if(type->is_struct())
		{
			struct STRUCT_TYPE* s=static_cast<struct STRUCT_TYPE*>(type);
			if(index < s->named_member_list.size())
			{
				member=s->named_member_list.at(index);
			}
		}else if(type->is_union())
		{
			struct UNION_TYPE* u=static_cast<struct UNION_TYPE*>(type);
			if(index < 1)
			{
				if(u->first_named_member!=NULL)
					member=u->first_named_member;
			}
		}
		if(member!=NULL)
		{
			if(member_offset!=NULL)
				*member_offset=member->offset;
			if(member->is_bit_field)
			{
				if(is_bit_field!=NULL)
					*is_bit_field=member->is_bit_field;
				if(bit_member!=NULL)
					*bit_member=member;
			}

			member_type= member->type;
		}
	}
	if(member_type!=NULL)
		member_type=member_type->unqualify();
	return member_type;
}
static struct INITIALIZER* next_initializer(std::vector<struct syntax_node*>::iterator *cur,std::vector<struct syntax_node*>::iterator *end)
{
	struct INITIALIZER* ini=NULL;
	if(*cur < *end)
	{
		ini=static_cast<struct INITIALIZER*>(**cur);
		(*cur)++;
	}
	return ini;
}


static void clear_mem(struct iVARIABLE* base_variable,unsigned int offset,unsigned int size)
{
	struct VAR* var=VAR::new_(base_variable,TYPE::pointer_type(TYPE::void_type()),offset);
	IC_CLEAR::new_(var,size);
}
static bool is_string_literal_ini_character_array(struct TYPE* type,struct INITIALIZER* ini)
{
	if(!type->is_array())
		return false;
	struct TYPE* member_type=static_cast<struct ARRAY_TYPE*>(type)->type_;
	if(!member_type->is_integral_type())
		return false;
	if(!static_cast<struct BASIC_TYPE*>(member_type)->is_int8() && !static_cast<struct BASIC_TYPE*>(member_type)->is_uint8())
		return false;
	if(is_compound(ini))
	{
		if(ini->list.size()>1)
			return false;
		ini=static_cast<struct INITIALIZER*>(ini->list.front());
		if(is_compound(ini))//still compound
			return false;
		struct exp* e=static_cast<struct exp*>(ini->value);
		if(e->op!="exp_STRING")
			return false;
	}else
	{
		struct exp* e=static_cast<struct exp*>(ini->value);
		if(e->op!="exp_STRING")
			return false;
	}
	return true;
}
static void string_literal_ini_character_array(struct TYPE* type,struct INITIALIZER* ini,unsigned int offset,struct INITIALIZER_DATA* ini_data)
{
	last_bit_field_unit_initialization(ini_data);
	padding(offset,ini_data);
	struct ARRAY_TYPE* a_type=static_cast<struct ARRAY_TYPE*>(type);
	if(is_compound(ini))
		ini=static_cast<struct INITIALIZER*>(ini->list.front());
	struct exp_STRING* str_exp=static_cast<struct exp_STRING*>(ini->value);
	std::string str_value(str_exp->lexical_value.begin()+1,str_exp->lexical_value.end()-1);
	str_value.append(1,'\0');
	if(a_type->number_of_element_==0)
	{
		a_type->number_of_element_=str_value.size();
	}
	if(ini_data->is_static)
	{
		ini_data->copy_action_string(offset,str_value);
	}else
	{
		ini->check();
		struct VAR* var=VAR::new_(ini_data->base_variable,TYPE::pointer_type(type),offset);
		IC_MOVEM::new_(var,ini->v_addr,str_value.size());
	}
	ini_data->last_ini_offset=offset+str_value.size();
}


static void check_union(struct TYPE* type,struct INITIALIZER* ini,
		std::vector<struct syntax_node*>::iterator *cur,
		std::vector<struct syntax_node*>::iterator *end,
		unsigned int offset,struct INITIALIZER_DATA* ini_data)
{
	if(is_compound(ini))
	{
		std::vector<struct syntax_node*>::iterator fst=ini->list.begin();
		std::vector<struct syntax_node*>::iterator end=ini->list.end();
		compound_check_initializer(type,&fst,&end,true,offset,ini_data);
	}else
	{
		if(cur!=NULL)
		{		
			ini->check();
			struct TYPE* fst_type=type;
			bool is_bit_field=false;
			struct iMEMBER* bit_member=NULL;
			while(fst_type!=NULL)
			{
				if(fst_type->is_compatible(ini->v_addr->type))
				{
					direct_check_initializer(fst_type,ini,offset,ini_data,is_bit_field,bit_member);
					break;
				}else if(is_aggregate_type(fst_type))
				{//try compound ini
					(*cur)--;//offset the first inc op
					/*
						cur and end is of outer compound,so set standalone=false;
					*/
					compound_check_initializer(fst_type,cur,end,false,offset,ini_data);
					break;
				}
				if(!fst_type->is_union())
				{
					::SCC_ERROR(ini->pos,"unassignable type");
					break;
				}
				//try again
				//if empty or zero named union
				if(static_cast<struct UNION_TYPE*>(fst_type)->first_named_member==NULL)
				{
					::SCC_ERROR(ini->pos,"too many initializers");
					break;
				}
				is_bit_field=static_cast<struct UNION_TYPE*>(fst_type)->first_named_member->is_bit_field;
				bit_member=is_bit_field?static_cast<struct UNION_TYPE*>(fst_type)->first_named_member:NULL;
				fst_type=static_cast<struct UNION_TYPE*>(fst_type)->first_named_member->type;
			}
		}else
		{
			direct_check_initializer(type,ini,offset,ini_data,false,NULL);
		}
	}
}
static void check_standard_unaggregate(struct TYPE* type,struct INITIALIZER* ini,
		unsigned int offset,struct INITIALIZER_DATA* ini_data,bool is_bit_field,struct iMEMBER* bit_member)
{
	if(is_compound(ini))
	{
		if(ini->list.size()>1)
		{
			::SCC_ERROR(ini->pos,"too many initializers");
		}
		ini=static_cast<struct INITIALIZER*>(ini->list.front());
	}
	if(is_compound(ini))
	{//after uncompund still compound.
		::SCC_ERROR(ini->pos,"compund initializer for unaggregate type");
	}else
	{
		direct_check_initializer(type,ini,offset,ini_data,is_bit_field,bit_member);
	}
}
static void check_aggregate(struct TYPE* type,struct INITIALIZER* ini,
		std::vector<struct syntax_node*>::iterator *cur,
		std::vector<struct syntax_node*>::iterator *end,
		unsigned int offset,struct INITIALIZER_DATA* ini_data)
{
	if(is_string_literal_ini_character_array(type,ini))
	{
		string_literal_ini_character_array(type,ini,offset,ini_data);
	}else
	{
		if(is_compound(ini))
		{
			std::vector<struct syntax_node*>::iterator fst=ini->list.begin();
			std::vector<struct syntax_node*>::iterator end=ini->list.end();
			compound_check_initializer(type,&fst,&end,true,offset,ini_data);
		}else
		{
			if(cur!=NULL)
			{
				ini->check();
				if(type->is_compatible(ini->v_addr->type))
				{
					direct_check_initializer(type,ini,offset,ini_data,false,NULL);
				}else
				{//try compound ini
					(*cur)--;//offset the first inc op
				/*
					cur and end is of outer compound,so set standalone=false;
				*/
					compound_check_initializer(type,cur,end,false,offset,ini_data);
				}
			}else
			{
				direct_check_initializer(type,ini,offset,ini_data,false,NULL);	
			}
		}
	}
}
static void last_bit_field_unit_initialization_static(struct INITIALIZER_DATA* ini_data)
{
	unsigned int temp=0;
	unsigned int offset=ini_data->last_ini_bit_offset;
	struct TYPE* type=ini_data->last_ini_bit_offset_type->unqualify();
	for(std::vector<std::pair<struct iMEMBER*,struct INITIALIZER*>>::iterator it=ini_data->bit_field_unit_ini_data.begin();
		it!=ini_data->bit_field_unit_ini_data.end();it++)
	{
		struct iMEMBER* member=it->first;
		struct INITIALIZER* ini=it->second;

		size_t prefix_n=member->bit_offset;
		unsigned int bmask=(1<<member->width)-1;
		unsigned int bmask_clear=~(bmask<<member->bit_offset);

		ini->check();
		if(exp_ASSIGN::can_assign(member->type,ini->v_addr))
		{
			if(ini->v_addr->is_CONST())
			{
				unsigned int value=static_cast<struct CONST*>(ini->v_addr)->as_uint32();
				temp=((value&bmask)<<(prefix_n)) | temp;
			}else
			{
				::SCC_ERROR(ini->pos,"initializer is not a constant");	
			}
		}else
		{
			::SCC_ERROR(ini->pos,"unassignable type");	
		}
	}
	struct CONST* con=new CONST(temp);
	ini_data->copy_action(offset,type,IC_CVT::cast_to(con,type));
	//ok,done with this unit,update last_ini_offset
	ini_data->last_ini_offset=offset+type->size;
}
static void last_bit_field_unit_initialization_automatic(struct INITIALIZER_DATA* ini_data)
{
	unsigned int temp=0;//all constant value;
	struct MEM* temp_mem=NULL;//all non-constant value;
	unsigned int offset=ini_data->last_ini_bit_offset;
	struct TYPE* type=ini_data->last_ini_bit_offset_type->unqualify();

	for(std::vector<std::pair<struct iMEMBER*,struct INITIALIZER*>>::iterator it=ini_data->bit_field_unit_ini_data.begin();
		it!=ini_data->bit_field_unit_ini_data.end();it++)
	{
		struct iMEMBER* member=it->first;
		struct INITIALIZER* ini=it->second;
		size_t prefix_n=member->bit_offset;
		unsigned int bmask=(1<<member->width)-1;

		ini->check();

		if(exp_ASSIGN::can_assign(member->type,ini->v_addr))
		{
			if(ini->v_addr->is_CONST())
			{
				unsigned int value=static_cast<struct CONST*>(ini->v_addr)->as_uint32();
				temp=((value&bmask)<<(prefix_n)) | temp;
			}else
			{
				ini->v_addr=IC_CVT::cast_to(ini->v_addr,member->type->unqualify());
				if(temp_mem==NULL)
				{
					temp_mem=MEM::local_temp(TYPE::basic_type("uint32"));
					struct CONST* con_zero=new CONST(0U);
					IC_MOVE::new_(temp_mem,con_zero);
				}
				struct ADDRESS* temp_value=IC_BITAND::new_(ini->v_addr,new CONST(bmask));
				temp_value=IC_LSHIFT::new_(temp_value,new CONST(prefix_n));
				temp_value=IC_BITOR::new_(temp_value,temp_mem);
				IC_MOVE::new_(temp_mem,temp_value);
			}
		}else
		{
			::SCC_ERROR(ini->pos,"unassignable type");	
		}
	}
	struct ADDRESS* result;
	if(temp_mem==NULL)
	{
		result=new CONST(temp);
	}else
	{
		if(temp!=0)
		{
			struct CONST* con=new CONST(temp);
			struct ADDRESS* temp_value=IC_BITOR::new_(con,temp_mem);
			result=temp_value;
		}else
			result=temp_mem;
	}

	struct VAR* var=VAR::new_(ini_data->base_variable,TYPE::pointer_type(type),offset);
	struct MEM* addr=new MEM(var);
	addr->type=type;
	addr->is_lvalue=type->is_object_type();
	IC_MOVE::new_(addr,IC_CVT::cast_to(result,addr->type));

	ini_data->last_ini_offset=offset+type->size;
}
static void last_bit_field_unit_initialization(struct INITIALIZER_DATA* ini_data)
{
	if(!ini_data->bit_field_unit_ini_data.empty())
	{
		if(ini_data->is_static)
		{
			last_bit_field_unit_initialization_static(ini_data);
		}else
		{
			last_bit_field_unit_initialization_automatic(ini_data);
		}
	}
	ini_data->bit_field_unit_ini_data.clear();
}
static void padding(unsigned int offset,struct INITIALIZER_DATA* ini_data)
{
	if(ini_data->last_ini_offset < offset)
	{
		if(ini_data->is_static)
		{
			ini_data->clear_action(ini_data->last_ini_offset,offset-ini_data->last_ini_offset);
		}else
		{
			clear_mem(ini_data->base_variable,ini_data->last_ini_offset,offset-ini_data->last_ini_offset);
		}
	}
}
static bool static_initialization_constant_address_data(bool is_neg,struct ADDRESS* addr,struct VAR** base_var,int* deta)
{
	if(addr->is_CONST())
	{
		struct CONST* con=static_cast<struct CONST*>(addr);
		if(is_neg)
		{
			*deta -=con->as_int32();
		}else
		{
			*deta +=con->as_int32();
		}
	}else if(addr->is_VAR())
	{
		struct VAR* var=static_cast<struct VAR*>(addr);
		if(var->var->storage!="static")
			return false;
		if(is_neg)
			return false;//negative var.
		if(*base_var!=NULL)
			return false;//double var.
		*base_var=var;
	}else if(addr->is_TEMP())
	{
		struct TEMP* temp=static_cast<struct TEMP*>(addr);
		if(temp->ins->is_addition())
		{

			return static_initialization_constant_address_data(is_neg,temp->ins->operand0,base_var,deta) && 
				static_initialization_constant_address_data(is_neg,temp->ins->operand1,base_var,deta);
		}else if(temp->ins->is_subtraction())
		{
			return static_initialization_constant_address_data(is_neg,temp->ins->operand0,base_var,deta) && 
				static_initialization_constant_address_data(!is_neg,temp->ins->operand1,base_var,deta);
		}else
			return false;//non add or sub

	}else
		return false;//mem or bit_field 
	return true;
}
static void direct_check_initializer(struct TYPE* type,struct INITIALIZER* ini,
	unsigned int offset,struct INITIALIZER_DATA* ini_data,bool is_bit_field,struct iMEMBER* bit_member)
{
	if(is_bit_field)
	{
		if(!ini_data->bit_field_unit_ini_data.empty() && ini_data->last_ini_bit_offset==offset)
		{
			ini_data->bit_field_unit_ini_data.push_back(std::pair<struct iMEMBER*,struct INITIALIZER*>(bit_member,ini));
		}else
		{//new field unit started!
			last_bit_field_unit_initialization(ini_data);
			padding(offset,ini_data);

			ini_data->last_ini_bit_offset_type=type;
			ini_data->last_ini_bit_offset=offset;
			ini_data->bit_field_unit_ini_data.push_back(std::pair<struct iMEMBER*,struct INITIALIZER*>(bit_member,ini));
		}
			
	}else
	{
		last_bit_field_unit_initialization(ini_data);
		padding(offset,ini_data);
		ini->check();
		if(exp_ASSIGN::can_assign(type,ini->v_addr))
		{
			ini->v_addr=IC_CVT::cast_to(ini->v_addr,type);
			if(ini_data->is_static)
			{
				struct VAR* base_var=NULL;
				int deta=0;
				if(static_initialization_constant_address_data(false,ini->v_addr,&base_var,&deta))
				{
					if(base_var==NULL)
					{
						ini_data->copy_action(offset,type,ini->v_addr);
					}else
					{
						ini_data->copy_action_base_deta(offset,type,base_var,deta);
					}
				}else
				{
					::SCC_ERROR(ini->pos,"initializer is not a constant");	
				}
			}else
			{
				assert(ini_data->base_variable!=NULL);
				struct VAR* var=VAR::new_(ini_data->base_variable,TYPE::pointer_type(type),offset);
				if(type->is_struct_or_union())
				{
					IC_MOVEM::new_(var,MEM::addr_of(ini->v_addr),type->size);
				}else
				{
					struct MEM* addr=new MEM(var);
					addr->type=type;
					addr->is_lvalue=type->is_object_type();
					IC_MOVE::new_(addr,ini->v_addr);
				}
			}
		}else
			::SCC_ERROR(ini->pos,"unassignable type");	
		ini_data->last_ini_offset=offset+type->size;
	}
}
static void compound_check_initializer(struct TYPE* type,
	std::vector<struct syntax_node*>::iterator *cur,
	std::vector<struct syntax_node*>::iterator *end,bool standalone,
	unsigned int offset,struct INITIALIZER_DATA* ini_data)
{
	unsigned int index_type=0;
	unsigned int member_offset=0;
	bool is_bit_field;
	struct iMEMBER* bit_member;
	struct TYPE* type_cur=member_of_index(type,index_type,&member_offset,&is_bit_field,&bit_member);
	member_offset += offset;
	struct INITIALIZER* ini_cur=next_initializer(cur,end);
//special case,for (empty/zero named) struct/union
	if(type_cur==NULL && standalone && ini_cur!=NULL)
	{
		::SCC_ERROR(ini_cur->pos,"too many initializers");
	}
	while(type_cur!=NULL)
	{
		if(is_aggregate_type(type_cur))
		{
			check_aggregate(type_cur,ini_cur,cur,end,member_offset,ini_data);
		}else if(type_cur->is_union())
		{
			check_union(type_cur,ini_cur,cur,end,member_offset,ini_data);
		}else
		{
			check_standard_unaggregate(type_cur,ini_cur,member_offset,ini_data,is_bit_field,bit_member);
		}
		//next member
		index_type++;
		type_cur=member_of_index(type,index_type,&member_offset,&is_bit_field,&bit_member);
		member_offset += offset;
		if(type_cur==NULL)
		{
			if(standalone)
			{
				ini_cur=next_initializer(cur,end);
				if(ini_cur!=NULL)
				{
					::SCC_ERROR(ini_cur->pos,"too many initializers");
				}
			}
			break;
		}else
		{
			ini_cur=next_initializer(cur,end);
			if(ini_cur==NULL)
			{
				break;
			}
		}
	}
}
struct INITIALIZER_DATA* INIT_DECLARATOR::check_initializer(struct TYPE* type,bool is_static,struct iVARIABLE* base_variable)
{
	type=type->unqualify();
	struct INITIALIZER* ini=static_cast<struct INITIALIZER*>(this->initializer_);
	if(ini==NULL)
		return NULL;

	struct INITIALIZER_DATA* ini_data=new struct INITIALIZER_DATA();
	ini_data->is_static=is_static;
	ini_data->base_variable=base_variable;

	if(is_aggregate_type(type))
	{
		check_aggregate(type,ini,NULL,NULL,0,ini_data);
	}else if(type->is_union())
	{
		check_union(type,ini,NULL,NULL,0,ini_data);
	}else
	{
		check_standard_unaggregate(type,ini,0,ini_data,false,NULL);
	}
	//complete the array.
	if(type->is_array() && type->is_incomplete())
	{
		struct ARRAY_TYPE* a=static_cast<struct ARRAY_TYPE*>(type);
		a->completion();
	}
	last_bit_field_unit_initialization(ini_data);
	padding(type->size,ini_data);
	return ini_data;
}
