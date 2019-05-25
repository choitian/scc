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

#include "type.h"
#include "scope.h"
#include "identifier.h"

struct TYPE* TYPE_BASE::unqualify()
{
	return this;
}
struct TYPE* TYPE_BASE::qualify(bool is_const,bool is_volatile)
{
	if( !is_const && !is_volatile)
		return this;
	else
	{
		return new struct QUALIFIED_TYPE(is_const,is_volatile,this);
	}
}

struct VOID_TYPE* TYPE::void_type()
{
	return new struct VOID_TYPE();
}
struct BASIC_TYPE* TYPE::basic_type(std::string op)
{
	return new struct BASIC_TYPE(op);
}
struct STRUCT_TYPE* TYPE::struct_type(std::string name)
{
	return new struct STRUCT_TYPE(name);
}
struct UNION_TYPE* TYPE::union_type(std::string name)
{
	return new struct UNION_TYPE(name);
}
struct ENUM_TYPE* TYPE::enum_type(std::string name)
{
	return new struct ENUM_TYPE(name);
}
struct POINTER_TYPE* TYPE::pointer_type(struct TYPE* type)
{
	return new struct POINTER_TYPE(type);
}

struct ARRAY_TYPE* TYPE::array_type(struct TYPE* type_,size_t number_of_element)
{
	return new struct ARRAY_TYPE(type_,number_of_element);
}
struct TYPE_FUNCTION* TYPE::type_function(struct TYPE* type_,struct SCOPE* prototype_scope_,bool is_variadic)
{
	return new struct TYPE_FUNCTION(type_,prototype_scope_,is_variadic);
}

void TYPE_PROTOTYPE::construct_prototype(struct SCOPE* prototype_scope_)
{
	prototype_scope_->clone_symbol_list(&this->list_);
}
bool TYPE_PROTOTYPE::is_compatible(struct TYPE* compare)
{
	if(compare==NULL)
		return false;
	compare=compare->unqualify();
	if(this->op==compare->op)
	{
		struct TYPE_PROTOTYPE *casted=static_cast<struct TYPE_PROTOTYPE *>(compare);
		if(this->list_.size()!=casted->list_.size())
			return false;
		if(this->is_variadic_!=casted->is_variadic_)
			return false;

		struct IDENTIFIER *parameter0,*parameter1;
		for(size_t i=0;i<this->list_.size();i++)
		{
			parameter0=this->list_.at(i);
			parameter1=casted->list_.at(i);
			if(!parameter0->type->is_compatible(parameter1->type))
				return false;
		}
		return true;
	}else
		return false;
}
static size_t ALIGN(size_t size,size_t align)
{
	return (size + align - 1) & (~(align - 1));
}
void STRUCT_TYPE::completion(struct SCOPE* member_scope)
{
	member_scope->clone_symbol_list(&this->member_list);
	for(size_t i=0;i<member_list.size();i++)
	{
		struct IDENTIFIER * member=member_list.at(i);
		if(!member->name.empty())
		{
			named_member_list.push_back(static_cast<struct iMEMBER*>(member));
			named_member_map_.insert(std::pair<std::string,struct IDENTIFIER *>(member->name,member));
		}
	}
	this->calculate_attribute();
	this->is_incomplete_=false;
}
bool STRUCT_TYPE::find_member(std::string name,struct iMEMBER** member)
{
	if(named_member_map_.find(name)!=named_member_map_.end())
	{
		if(member!=NULL)
			*member=static_cast<struct iMEMBER*>(named_member_map_[name]);
		return true;
	}
	return false;
}
void STRUCT_TYPE::calculate_attribute()
{
	size_t offset=0;
	size_t align=0;
	struct iMEMBER* member;
	size_t member_align=0;

	struct TYPE* last_field_type=NULL;
	size_t last_field_offset=0;
	size_t remain_field_bits=0;
	for(size_t i=0;i<this->member_list.size();i++)
	{
		member=static_cast<struct iMEMBER*>(this->member_list.at(i));
		if(member->type->is_const())
			this->with_const_member_=true;
		else if(member->type->is_struct())
		{
			struct STRUCT_TYPE* st=static_cast<struct STRUCT_TYPE*>(member->type);
			if(st->with_const_member_)
				this->with_const_member_=true;
		}else if(member->type->is_union())
		{
			struct UNION_TYPE* ut=static_cast<struct UNION_TYPE*>(member->type);
			if(ut->with_const_member_)
				this->with_const_member_=true;
		}
		if(member->is_bit_field)
		{
			if(member->width==0 && member->name.empty())
			{//force padding
				remain_field_bits=0;
			}else
			{
				//start a new unit if needed.
				if(	remain_field_bits < member->width ||//don't split between units.
					last_field_type->size!=member->type->size)//don't use un-natual units,
					//e.g. 'long' can in 'unsigned long' but not 'short','char'.
				{
					member_align=member->type->align;
					offset=ALIGN(offset,member_align);
					if(member_align > align)
						align=member_align;
					last_field_offset=offset;
					offset+=member->type->size;

					last_field_type=member->type;
					remain_field_bits=member->type->size*8;;
				}
				member->offset     = last_field_offset;
				member->bit_offset = (last_field_type->size*8)-remain_field_bits;
				remain_field_bits -= member->width;
			}
		}else
		{
			remain_field_bits=0;//force padding field if any.

			member_align=member->type->align;
			offset=ALIGN(offset,member_align);
			if(member_align > align)
				align=member_align;
			member->offset=offset;//set member's offet
			offset=offset+member->type->size;
		}
	}
	this->align=align;
	this->size=ALIGN(offset,align);
}
void UNION_TYPE::completion(struct SCOPE* member_scope)
{
	member_scope->clone_symbol_list(&this->member_list);
	for(size_t i=0;i<member_list.size();i++)
	{
		struct IDENTIFIER * member=member_list.at(i);
		if(!member->name.empty())
		{
			named_member_list.push_back(static_cast<struct iMEMBER*>(member));
			named_member_map_.insert(std::pair<std::string,struct IDENTIFIER *>(member->name,member));
		}
	}
	if(!named_member_list.empty())
	{
		this->first_named_member=named_member_list[0];
	}else
		this->first_named_member=NULL;
	this->calculate_attribute();
	this->is_incomplete_=false;
}
bool UNION_TYPE::find_member(std::string name,struct iMEMBER** member)
{
	if(named_member_map_.find(name)!=named_member_map_.end())
	{
		if(member!=NULL)
			*member=static_cast<struct iMEMBER*>(named_member_map_[name]);
		return true;
	}
	return false;
}
void UNION_TYPE::calculate_attribute()
{
	size_t max_size=0;
	size_t max_align=0;
	struct iMEMBER* member;
	for(size_t i=0;i<this->member_list.size();i++)
	{
		member=static_cast<struct iMEMBER*>(this->member_list.at(i));
		if(member->type->is_const())
			this->with_const_member_=true;
		else if(member->type->is_struct())
		{
			struct STRUCT_TYPE* st=static_cast<struct STRUCT_TYPE*>(member->type);
			if(st->with_const_member_)
				this->with_const_member_=true;
		}else if(member->type->is_union())
		{
			struct UNION_TYPE* ut=static_cast<struct UNION_TYPE*>(member->type);
			if(ut->with_const_member_)
				this->with_const_member_=true;
		}
		if(member->type->align > max_align)
			max_align=member->type->align;
		if(member->type->size >max_size)
			max_size=member->type->size;
	}
	this->align=max_align;
	this->size=ALIGN(max_size,max_align);
}
