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

struct TYPE
{
	size_t size;
	size_t align;
	std::string op;
	static struct VOID_TYPE* void_type();
	static struct BASIC_TYPE* basic_type(std::string op);
	static struct STRUCT_TYPE* struct_type(std::string name);
	static struct UNION_TYPE* union_type(std::string name);
	static struct ENUM_TYPE* enum_type(std::string name);
	static struct POINTER_TYPE* pointer_type(struct TYPE* type);
	static struct ARRAY_TYPE* array_type(struct TYPE* type_,size_t number_of_element);
	static struct TYPE_FUNCTION* type_function(struct TYPE* type_,struct SCOPE* prototype_scope_,bool is_variadic);


	virtual struct TYPE* unqualify()=0;
	virtual struct TYPE* qualify(bool is_const,bool is_volatile)=0;
	virtual bool is_compatible(struct TYPE* type)=0;
	virtual bool is_const()	=0;
	virtual bool is_volatile() =0;

	virtual bool is_struct_or_union()=0;
	virtual bool is_enum()=0;
	virtual bool is_union()=0;
	virtual bool is_struct()=0;
	virtual bool is_function()=0;
	virtual bool is_pointer()=0;
	virtual bool is_pointer_to_object_type()=0;
	virtual bool is_array() =0;
	virtual bool is_void()=0;
	virtual bool is_object_type()=0;
	virtual bool is_incomplete()=0;
	virtual bool is_arithmetic_type()=0;
	virtual bool is_integral_type()=0;
	virtual bool is_floating_type()=0;
	virtual bool is_scalar_type()=0;
	virtual bool is_unsigned()=0;
};
struct QUALIFIED_TYPE:public TYPE
{
	QUALIFIED_TYPE(bool is_const,bool is_volatile,struct TYPE *qualified)
	{
		this->op="QUALIFIED";
		this->is_const_=is_const;
		this->is_volatile_=is_volatile;
		assert(qualified!=NULL);
		this->type_=qualified;
		this->size=qualified->size;
		this->align=qualified->align;
	}

	virtual struct TYPE* unqualify(){
		return this->type_;
	}
	virtual struct QUALIFIED_TYPE* qualify(bool is_const,bool is_volatile)
	{
		if(is_const)
			this->is_const_=true;
		if(is_volatile)
			this->is_volatile_=true;
		return this;
	}

	virtual bool is_const()	  {	return is_const_;}
	virtual bool is_volatile(){	return is_volatile_;}
	virtual bool is_compatible(struct TYPE* compare)
	{
		if(compare==NULL)
			return false;
		compare=compare->unqualify();
		return type_->is_compatible(compare);
	}
	virtual bool is_struct_or_union()   {	 return type_->is_struct_or_union(); }
	virtual bool is_enum()   {	return type_->is_enum();}
	virtual bool is_union()   {	return type_->is_union();}
	virtual bool is_struct()  {	return type_->is_struct();}
	virtual bool is_function(){	return type_->is_function();}
	virtual bool is_pointer() {	return type_->is_pointer();}
	virtual bool is_pointer_to_object_type() {	return type_->is_pointer_to_object_type();}
	virtual bool is_array()   {	return type_->is_array();}
	virtual bool is_void()	  {	return type_->is_void();}
	virtual bool is_object_type()	{	return type_->is_object_type();}
	virtual bool is_incomplete()	{	return type_->is_incomplete();}
	virtual bool is_arithmetic_type()   {	return type_->is_arithmetic_type();}
	virtual bool is_integral_type()   {	return type_->is_integral_type();}
	virtual bool is_floating_type()  {return type_->is_floating_type();}
	virtual bool is_scalar_type()   {	return type_->is_scalar_type();}
	virtual bool is_unsigned() {	return type_->is_unsigned();}
private:
	struct TYPE *type_;
	bool is_const_;
	bool is_volatile_;
};
struct TYPE_BASE:public TYPE
{
	virtual struct TYPE* unqualify();
	virtual struct TYPE* qualify(bool is_const,bool is_volatile);

	virtual bool is_const()	      {	return false;}
	virtual bool is_volatile()    {	return false;}

	virtual bool is_struct_or_union()   {	return is_union()||is_struct();}
	virtual bool is_enum()   {	return false;}
	virtual bool is_union()   {	return false;}
	virtual bool is_struct()  {	return false;}
	virtual bool is_function(){	return false;}
	virtual bool is_pointer() {	return false;}
	virtual bool is_pointer_to_object_type() {	return false;}
	virtual bool is_array()   {	return false;}
	virtual bool is_void()	  {	return false;}
	virtual bool is_object_type()	{return true;}
	virtual bool is_incomplete()	{return false;}
	virtual bool is_arithmetic_type() {	return false;}
	virtual bool is_integral_type()   {	return false;}
	virtual bool is_floating_type()  { return false;}
	virtual bool is_scalar_type()     {	return false;}
	virtual bool is_unsigned()		{	return false;}
};



struct VOID_TYPE:public TYPE_BASE
{
	VOID_TYPE()
	{
		op=="VOID";
		this->align=this->size=0;
	}
	virtual bool is_compatible(struct TYPE* compare)
	{
		if(compare==NULL)
			return false;
		compare=compare->unqualify();
		return this->op==compare->op;
	}
	virtual bool is_void()	  {	return true;}
	virtual bool is_object_type()	{return false;}
	virtual bool is_incomplete()	{return true;}

};
struct BASIC_TYPE:public TYPE_BASE
{
	BASIC_TYPE(std::string op)
	{
		this->op=op;
		if(op=="int8" || op=="uint8")
		{
			this->align=this->size=1;
		}else if(op=="int16" || op=="uint16")
		{
			this->align=this->size=2;
		}else if(op=="int32" || op=="uint32")
		{
			this->align=this->size=4;
		}else if(op=="float32")
		{
			this->align=this->size=4;
		}else if(op=="float64")
		{
			this->align=this->size=8;
		}else
			assert(0);
	}
	virtual bool is_compatible(struct TYPE* compare)
	{
		if(compare==NULL)
			return false;
		compare=compare->unqualify();
		return this->op==compare->op;
	}
	virtual bool is_arithmetic_type()   {	return true;}
	virtual bool is_integral_type()   {	return op!="float32" && op!="float64";}
	virtual bool is_floating_type()  { return !is_integral_type();}
	virtual bool is_scalar_type()     {	return true;}
	virtual bool is_unsigned()		{	return is_uint8() || is_uint16() || is_uint32();}

	bool is_int8() {return op=="int8";}
	bool is_uint8() {return op=="uint8";}
	bool is_int16() {return op=="int16";}
	bool is_uint16() {return op=="uint16";}
	bool is_int32() {return op=="int32";}
	bool is_uint32() {return op=="uint32";}
	bool is_float32() {return op=="float32";}
	bool is_float64() {return op=="float64";}
};
struct STRUCT_TYPE:public TYPE_BASE
{
	STRUCT_TYPE(std::string name){
		static int id=0;

		this->op="STRUCT";
		this->size=this->align=0;
		this->id_=id++;
		this->name_=name;
		this->is_incomplete_=true;
		this->with_const_member_=false;
	}

	virtual bool is_compatible(struct TYPE* compare)
	{
		if(compare==NULL)
			return false;
		compare=compare->unqualify();
		if(compare->op==this->op)
		{
			struct STRUCT_TYPE *casted=static_cast<struct STRUCT_TYPE *>(compare);
			return id_==casted->id_;
		}else
			return false;
	}
	virtual bool is_incomplete(){return is_incomplete_;}
	virtual bool is_struct()  {	return true; }

	void completion(struct SCOPE* member_scope);
	bool find_member(std::string name,struct iMEMBER** member);
	bool with_const_member_;
	std::vector<struct iMEMBER *> named_member_list;
private:
	void calculate_attribute();
	std::vector<struct IDENTIFIER *> member_list;
	std::map<std::string,struct IDENTIFIER *> named_member_map_;
	std::string name_;
	int id_;
	bool is_incomplete_;

};
struct UNION_TYPE:public TYPE_BASE
{
	UNION_TYPE(std::string name){
		static int id=0;

		this->op="UNION";
		this->size=this->align=0;
		this->id_=id++;
		this->name_=name;
		this->is_incomplete_=true;
		this->with_const_member_=false;
	}

	virtual bool is_compatible(struct TYPE* compare)
	{
		if(compare==NULL)
			return false;
		compare=compare->unqualify();
		if(compare->op==this->op)
		{
			struct UNION_TYPE *casted=static_cast<struct UNION_TYPE *>(compare);
			return id_==casted->id_;
		}else
			return false;
	}
	virtual bool is_incomplete(){	return is_incomplete_;}
	virtual bool is_union()		{	return true;}

	void completion(struct SCOPE* member_scope);
	bool find_member(std::string name,struct iMEMBER** member);
	bool with_const_member_;
	std::vector<struct iMEMBER *> named_member_list;
	struct iMEMBER * first_named_member;
private:
	void calculate_attribute();
	std::vector<struct IDENTIFIER *> member_list;
	std::map<std::string,struct IDENTIFIER *> named_member_map_;
	std::string name_;
	int id_;
	bool is_incomplete_;
};
struct ENUM_TYPE:public TYPE_BASE
{
	ENUM_TYPE(std::string name){
		static int id=0;

		this->op="ENUM";
		this->size=this->align=4;
		this->id_=id++;
		this->name_=name;
		this->defined=false;
	}

	virtual bool is_compatible(struct TYPE* compare)
	{
		assert(0);//should never be called.
		return false;
	}
	virtual bool is_enum()		{	return true;}
	bool defined;
	std::vector<struct iVARIABLE*> member_list;
private:
	std::string name_;
	int id_;
};
struct POINTER_TYPE:public TYPE_BASE
{
	POINTER_TYPE(struct TYPE* type_)
	{
		op="PTR";
		this->align=this->size=4;
		this->type_=type_;
	};
	virtual bool is_compatible(struct TYPE* compare)
	{
		if(compare==NULL)
			return false;
		compare=compare->unqualify();
		if(this->op==compare->op)
		{
			struct POINTER_TYPE *casted=static_cast<struct POINTER_TYPE *>(compare);
			return this->type_->is_compatible(casted->type_);
		}else
			return false;
	}
	virtual bool is_scalar_type()     {	return true;}
	virtual bool is_pointer(){	return true;}
	virtual bool is_pointer_to_object_type() {return type_->is_object_type();}
	struct TYPE *type_;
};
struct ARRAY_TYPE:public TYPE_BASE
{
	ARRAY_TYPE(struct TYPE* type_,size_t number_of_element)
	{
		op="ARRAY";
		this->number_of_element_=number_of_element;
		this->size=number_of_element* type_->size;
		this->align=type_->align;
		this->type_=type_;
		this->is_incomplete_=(number_of_element==0);
	};
	virtual bool is_compatible(struct TYPE* compare)
	{
		if(compare==NULL)
			return false;
		compare=compare->unqualify();
		if(this->op==compare->op)
		{
			struct ARRAY_TYPE *casted=static_cast<struct ARRAY_TYPE *>(compare);
			if(this->number_of_element_==0|| casted->number_of_element_==0)
				return this->type_->is_compatible(casted->type_);
			else if(this->number_of_element_==casted->number_of_element_)
				return this->type_->is_compatible(casted->type_);
			else
				return false;
		}else
			return false;
	}
	virtual bool is_incomplete() { return this->is_incomplete_;}
	bool is_incomplete_;
	void completion()
	{
		is_incomplete_=false;
		this->size=number_of_element_* type_->size;		
	}
	virtual bool is_array(){	return true;}
	struct TYPE *type_;
	size_t number_of_element_;
};
struct TYPE_PROTOTYPE:public TYPE_BASE
{
	TYPE_PROTOTYPE(struct SCOPE* prototype_scope_,bool is_variadic)
	{
		op="PROTOTYPE";
		this->align=this->size=0;
		this->is_variadic_=is_variadic;
		construct_prototype(prototype_scope_);
	}
	virtual bool is_compatible(struct TYPE* compare);
	virtual bool is_incomplete() { return false; }
	virtual bool is_object_type(){ return false;}	

	bool is_variadic_;
	std::vector<struct IDENTIFIER*> list_;
	void construct_prototype(struct SCOPE* prototype_scope_);
};
struct TYPE_FUNCTION:public TYPE_BASE
{
	TYPE_FUNCTION(struct TYPE* type_,struct SCOPE* prototype_scope_,bool is_variadic)
	{
		op="FUNCTION";
		this->align=this->size=0;
		this->type_=type_;
		this->calling_convention_="cdecl";//default calling_convention;

		this->prototype_scope_=prototype_scope_;
		this->prototype_=new struct TYPE_PROTOTYPE(prototype_scope_,is_variadic);
	};
	virtual bool is_compatible(struct TYPE* compare)
	{
		if(compare==NULL)
			return false;
		compare=compare->unqualify();
		if(this->op==compare->op)
		{
			struct TYPE_FUNCTION *casted=static_cast<struct TYPE_FUNCTION *>(compare);
			return this->type_->is_compatible(casted->type_) && this->prototype_->is_compatible(casted->prototype_);
		}else
			return false;
	}
	virtual bool is_incomplete() { return false; }
	virtual bool is_object_type(){return false; }

	virtual bool is_function(){	return true;}

	void set_calling_convention(std::string calling_convention)
	{
		this->calling_convention_=calling_convention;
	}

	struct TYPE *type_;
	std::string calling_convention_;
	struct TYPE_PROTOTYPE* prototype_;
	struct SCOPE* prototype_scope_;
};
