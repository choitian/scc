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


void STRUCT_DECLARATOR::install_member(struct TYPE *type,std::string name)
{
	struct IDENTIFIER* scope_symbol_old;
	scope_symbol_old=SCOPE::current->scope_find(name);
	if(scope_symbol_old!=NULL)
	{
		::SCC_ERROR(pos,"member : %s : redefinition.",name.c_str());
	}else
	{
		struct iMEMBER* for_install=IDENTIFIER::member(type,name);
		SCOPE::current->insert(name,for_install);
	}
}
void STRUCT_DECLARATOR::install_member_bit_field(struct TYPE *type,std::string name,size_t width)
{
	if(name.empty())
	{
		struct iMEMBER* for_install=IDENTIFIER::member(type,name);
		for_install->is_bit_field=true;
		for_install->width=width;
		SCOPE::current->insert(name,for_install);
	}else
	{
		struct IDENTIFIER* scope_symbol_old;
		scope_symbol_old=SCOPE::current->scope_find(name);
		if(scope_symbol_old!=NULL)
		{
			::SCC_ERROR(pos,"member : %s : redefinition.",name.c_str());
		}else
		{
			struct iMEMBER* for_install=IDENTIFIER::member(type,name);
			for_install->is_bit_field=true;
			for_install->width=width;
			SCOPE::current->insert(name,for_install);
		}
	}
}
void STRUCT_DECLARATOR::check(struct TYPE *base_type)
{
	std::string name;
	struct TYPE* type=base_type;
	if(declaration_declarator_!=NULL)
	{
		declaration_declarator_->check(base_type);
		assert(!declaration_declarator_->identifier.empty());
		name=declaration_declarator_->identifier;
		type=declaration_declarator_->type;
	}
	if(this->width!=NULL)
	{
		size_t field_width;
		if(!type->is_integral_type())
		{
			::SCC_ERROR(pos,"%s : type of bit field must be integral.",name.empty()?"member":name.c_str());
			return;
		}
		struct CONST* con=CONST::constant_expression_2_const(static_cast<struct exp*>(this->width));
		if(con!=NULL)
		{
			if(con->type->is_integral_type())
				field_width=con->as_uint32();
			else
			{
				::SCC_ERROR(pos,"constant expression is not integral.");
				return;
			}
		}else
		{
			::SCC_ERROR(pos,"expected constant expression.");
			return;
		}
		if(type->size*8 <field_width)
		{
			::SCC_ERROR(pos,"%s : type of bit field too small for number of bits.",name.empty()?"member":name.c_str());
			return;
		}
		if(field_width==0 && !name.empty())
		{
			::SCC_ERROR(pos,"%s : named bit field cannot have zero width.",name.c_str());
			return;
		}
		install_member_bit_field(type,name,field_width);
	}else
	{
		if(type->is_incomplete())
		{
			if(type->is_array())
			{
				::SCC_ERROR(pos,"%s : unknown size.",name.c_str());
			}else if(type->is_void())
				::SCC_ERROR(pos,"illegal use of type 'void'.");
			else
				::SCC_ERROR(pos,"incomplete struct or union.");
		}else if(type->is_function())
			::SCC_ERROR(pos,"%s : member has function type.",name.c_str());
		else
			install_member(type,name);
	}
}
void STRUCT_DECLARATION::check()
{
	this->declaration_type_->check(false);
	for(size_t i=0;i< this->list.size();i++)
	{
		struct syntax_node* node=list.at(i);
		if(node->op=="STRUCT_DECLARATOR")
		{
			struct STRUCT_DECLARATOR* casted=static_cast<struct STRUCT_DECLARATOR*>(node);
			casted->check(declaration_type_->type);
		}
		else
			assert(0);
	}
}
void TYPE_STRUCT::check_definition()
{
	if(!this->name.empty())
	{
		this->type=SCOPE::current->scope_find_tag(this->name);
		if(this->type!=NULL)
		{
			if(!this->type->is_struct())
				::SCC_ERROR(pos,"struct : %s : redefinition.",name.c_str());
			if(!this->type->is_incomplete())
				::SCC_ERROR(pos,"struct : %s : redefinition.",name.c_str());
		}else
		{
			this->type=TYPE::struct_type(this->name);
			SCOPE::current->insert_tag(this->name,this->type);
		}
	}else
	{
		this->type=TYPE::struct_type(this->name);
		SCOPE::current->insert_tag(this->name,this->type);
	}
	SCOPE::current->enter_scope();
	struct SCOPE* member_scope=SCOPE::current;
	{
		struct STRUCT_DECLARATION* member;
		for(size_t i=0;i<this->list.size();i++)
		{
			member=static_cast<struct STRUCT_DECLARATION*>(this->list.at(i));
			member->check();
		}
		struct STRUCT_TYPE* struct_type=static_cast<struct STRUCT_TYPE*>(this->type);

		struct_type->completion(member_scope);
	}
	SCOPE::current->exit_scope();
}
void TYPE_STRUCT::check_declaration(bool need_new_tag)
{

	if(need_new_tag)
	{//only checking current scope
		this->type=SCOPE::current->scope_find_tag(this->name);
		if(this->type!=NULL)
		{
			if(!this->type->is_struct())
				::SCC_ERROR(pos,"struct : %s : redefinition.",name.c_str());

		}else
		{
			this->type=TYPE::struct_type(this->name);
			SCOPE::current->insert_tag(this->name,this->type);
		}
	}else
	{//here just using this tag as struct.
		this->type=SCOPE::current->find_tag(this->name);
		if(this->type!=NULL)
		{
			if(!this->type->is_struct())
			{
				::SCC_ERROR(pos,"%s : using as struct tag.",name.c_str());
			}
		}else
		{
			this->type=TYPE::struct_type(this->name);
			SCOPE::current->insert_tag(this->name,this->type);
		}
	}
}
void TYPE_STRUCT::check(bool need_new_tag)
{
	if(!this->list.empty())
	{
		check_definition();
	}else
	{
		check_declaration(need_new_tag);
	}
}

void TYPE_UNION::check_definition()
{
	if(!this->name.empty())
	{
		this->type=SCOPE::current->scope_find_tag(this->name);
		if(this->type!=NULL)
		{
			if(!this->type->is_union())
				::SCC_ERROR(pos,"union : %s : redefinition.",name.c_str());
			if(!this->type->is_incomplete())
				::SCC_ERROR(pos,"union : %s : redefinition.",name.c_str());
		}else
		{
			this->type=TYPE::union_type(this->name);
			SCOPE::current->insert_tag(this->name,this->type);
		}
	}else
	{
		this->type=TYPE::union_type(this->name);
		SCOPE::current->insert_tag(this->name,this->type);
	}
	SCOPE::current->enter_scope();
	struct SCOPE* member_scope=SCOPE::current;
	{
		struct STRUCT_DECLARATION* member;
		for(size_t i=0;i<this->list.size();i++)
		{
			member=static_cast<struct STRUCT_DECLARATION*>(this->list.at(i));
			member->check();
		}
		struct UNION_TYPE* union_type=static_cast<struct UNION_TYPE*>(this->type);

		union_type->completion(member_scope);
	}
	SCOPE::current->exit_scope();
}
void TYPE_UNION::check_declaration(bool need_new_tag)
{
	if(need_new_tag)
	{//only checking current scope
		this->type=SCOPE::current->scope_find_tag(this->name);
		if(this->type!=NULL)
		{
			if(!this->type->is_union())
				::SCC_ERROR(pos,"union : %s : redefinition.",name.c_str());

		}else
		{
			this->type=TYPE::union_type(this->name);
			SCOPE::current->insert_tag(this->name,this->type);
		}
	}else
	{//here just using this tag as struct.
		this->type=SCOPE::current->find_tag(this->name);
		if(this->type!=NULL)
		{
			if(!this->type->is_union())
			{
				::SCC_ERROR(pos,"%s : using as union tag.",name.c_str());
			}
		}else
		{
			this->type=TYPE::union_type(this->name);
			SCOPE::current->insert_tag(this->name,this->type);
		}
	}
}
void TYPE_UNION::check(bool need_new_tag)
{
	if(!this->list.empty())
	{
		check_definition();
	}else
	{
		check_declaration(need_new_tag);
	}
}

void TYPE_ENUM::check_definition()
{
	if(!this->name.empty())
	{
		this->type=SCOPE::current->scope_find_tag(this->name);
		if(this->type!=NULL)
		{
			if(!this->type->is_enum())
			{
				::SCC_ERROR(pos,"enum : %s : redefinition.",name.c_str());
			}else
			{
				struct ENUM_TYPE* enum_type=static_cast<struct ENUM_TYPE*>(this->type);
				if(enum_type->defined)
					::SCC_ERROR(pos,"enum : %s : redefinition.",name.c_str());
			}
		}else
		{
			this->type=TYPE::enum_type(this->name);
			SCOPE::current->insert_tag(this->name,this->type);
		}
	}else
	{
		this->type=TYPE::enum_type(this->name);
		SCOPE::current->insert_tag(this->name,this->type);
	}
	struct ENUM_TYPE* enum_type=static_cast<struct ENUM_TYPE*>(this->type);
	{
		enum_type->defined=true;
		struct ENUMERATOR* member;
		std::string name;
		int value=0;
		for(size_t i=0;i<this->list.size();i++)
		{
			member=static_cast<struct ENUMERATOR*>(this->list.at(i));
			name=member->name;
			struct IDENTIFIER* scope_symbol_old;
			scope_symbol_old=SCOPE::current->scope_find(name);
			if(scope_symbol_old!=NULL)
			{
				::SCC_ERROR(pos,"%s : redefinition.",name.c_str());
			}else
			{
				if(member->initializer!=NULL)
				{
					struct CONST* con=CONST::constant_expression_2_const(static_cast<struct exp*>(member->initializer));
					if(con!=NULL)
					{
						if(con->type->is_integral_type())
							value=con->as_int32();
						else
							::SCC_ERROR(pos,"constant expression is not integral.");
					}else
						::SCC_ERROR(pos,"expected constant expression.");			
				}

				struct iVARIABLE* for_install=IDENTIFIER::variable_enum_constant(enum_type,name,value++);
				SCOPE::current->insert(name,for_install);
				enum_type->member_list.push_back(for_install);
			}
		}
	}
}
void TYPE_ENUM::check_declaration()
{
	this->type=SCOPE::current->find_tag(this->name);
	if(this->type!=NULL)
	{
		if(!this->type->is_enum())
		{
			std::string old=this->type->is_struct()?"struct":"union";
			::SCC_ERROR(pos,"%s : using as enum tag,already as %s tag",name.c_str(),old.c_str());
		}
	}else
	{
		this->type=TYPE::enum_type(this->name);
		SCOPE::current->insert_tag(this->name,this->type);
	}
}
void TYPE_ENUM::check()
{
	if(!this->list.empty())
	{
		check_definition();
	}else
	{
		check_declaration();
	}
	this->type=TYPE::basic_type("int32");//as int
}
struct TYPE * DECLARATION_TYPE::typedef_name_type(std::string name)
{
	struct IDENTIFIER* symbol;
	symbol=SCOPE::current->find(name);
	if(symbol!=NULL && symbol->kind=="TYPEDEF_NAME")
	{
		return symbol->type;
	}else
	{
		::SCC_ERROR(pos,"%s :does not name a type.",name.c_str());
		return NULL;
	}
}
void DECLARATION_TYPE::check(bool need_new_tag)
{
	std::string type;
	std::string sign;
	std::string length;
	std::string sclass;
	struct TYPE_STRUCT *type_struct = NULL;
	struct TYPE_UNION *type_union = NULL;
	struct TYPE_ENUM *type_enum = NULL;
	struct TYPE *type_of_typedef_name = NULL;
	bool see_const=false;
	bool see_volatile=false;
//get specifier_list information
	for(size_t i=0;i<list.size();i++)
	{
		struct syntax_node* node=list.at(i);
		std::string tok,extra;
		if(node->is_terminal(&tok,&extra))
		{
			if(tok=="auto" || tok=="register" || tok=="static" || tok=="extern" ||tok=="typedef")
			{
				if(!sclass.empty())
					::SCC_ERROR(pos,"illegal specifier combination.");
				sclass=tok;
			}else if(tok=="void" || tok=="char" || tok=="int" ||tok=="float" ||tok=="double")
			{
				if(!type.empty())
					::SCC_ERROR(pos,"illegal specifier combination.");
				type=tok;

			}else if(tok=="short" || tok=="long")
			{
				if(!length.empty())
					::SCC_ERROR(pos,"illegal specifier combination.");
				length=tok;
			}else if(tok=="unsigned" || tok=="signed")
			{
				if(!sign.empty())
					::SCC_ERROR(pos,"illegal specifier combination.");
				sign=tok;
			}else if(tok=="const")
			{
				see_const=true;
			}else if(tok=="volatile")
			{
				see_volatile=true;
			}else if(tok=="typedef_name")
			{
				if(!type.empty())
					::SCC_ERROR(pos,"illegal specifier combination.");
				type_of_typedef_name=this->typedef_name_type(extra);
				if(type_of_typedef_name!=NULL)
					type="typedef_name";
			}
		}else if(node->op=="TYPE_STRUCT")
		{
			if(!type.empty())
				::SCC_ERROR(pos,"illegal specifier combination.");
			type="struct";

			type_struct=static_cast<struct TYPE_STRUCT *>(node);
			type_struct->check(need_new_tag);
		}else if(node->op=="TYPE_UNION")
		{
			if(!type.empty())
				::SCC_ERROR(pos,"illegal specifier combination.");
			type="union";

			type_union=static_cast<struct TYPE_UNION *>(node);
			type_union->check(need_new_tag);
		}else if(node->op=="TYPE_ENUM")
		{
			if(!type.empty())
				::SCC_ERROR(pos,"illegal specifier combination.");
			type="enum";

			type_enum=static_cast<struct TYPE_ENUM *>(node);
			type_enum->check();
		}else
			assert(0);
	}

//check length,sign,miss type specifier
	if(type.empty())
	{
		if(!length.empty())
			type="int";
		else
		{
			if(sign.empty())
				::SCC_WARNNING(pos,"missing explicit type; default as int");
			type="int";
		}
	}
	if(!sign.empty())
	{
		if(type!="char" && type!="int")
			::SCC_ERROR(pos,"illegal specifier combination.");
	}
	if(!length.empty())
	{
		if(type!="int" && type!="double")
			::SCC_ERROR(pos,"illegal specifier combination.");
	}
//get type system name.
	if(type=="void")
	{
		type=="void";
	}else if(type=="char")
	{
		if(sign=="unsigned")
			type="uint8";
		else
			type="int8";
	}else if(type=="int")
	{
		if(sign=="unsigned")
		{
			type=length=="short"?"uint16":"uint32";
		}
		else
		{
			type=length=="short"?"int16":"int32";
		}
	}else if(type=="float")
	{
		type="float32";
	}else if(type=="double")
	{
		type="float64";
	}
//result type
	if(type=="struct")
	{
		this->type=type_struct->type;
	}else if(type=="union")
	{
		this->type=type_union->type;
	}else if(type=="enum")
	{
		this->type=type_enum->type;
	}else if(type=="void")
	{
		this->type=TYPE::void_type();
	}else if(type=="typedef_name")
	{
		this->type=type_of_typedef_name;
	}
	else
	{
		this->type=TYPE::basic_type(type);
	}
//qualifying
	this->type=this->type->qualify(see_const,see_volatile);

//storage class
	this->sclass=sclass;
}
void TRANSLATION_UNIT::check()
{
	SCOPE::start_file_scope();
	struct syntax_node* node;
	for(size_t i=0;i< this->list.size();i++)
	{
		node=list.at(i);
		if(node->op=="FUNCTION_DEFINITION")
		{
			struct FUNCTION_DEFINITION*  casted=static_cast<struct FUNCTION_DEFINITION*>(node);
			this->function_definition_list.push_back(casted);
			casted->check();
		}else if(node->op=="DECLARATION")
		{
			struct DECLARATION*  casted=static_cast<struct DECLARATION*>(node);
			casted->check();
		}else
		{
			assert(0);
		}
	}
}

void FUNCTION_DEFINITION::check_body_label(struct iFUNCTION* funtion_symbol)
{
	for(size_t i=0;i<funtion_symbol->function_scope.label_list.size();i++)
	{
		struct iLABEL *label=funtion_symbol->function_scope.label_list.at(i);
		if(!label->defined)
		{
			::SCC_ERROR(pos,"%s : undefined label.",label->name.c_str());
		}
	}
}
void FUNCTION_DEFINITION::check_body(struct iFUNCTION* current_funtion_symbol)
{
	//reset current function data.
	iFUNCTION::current_definition=current_funtion_symbol;

	struct TYPE_FUNCTION* function_type=static_cast<struct TYPE_FUNCTION*>(declaration_declarator_->type);
	struct stmt_COMPOUND* body=static_cast<struct stmt_COMPOUND*>(this->body);
	body->funtion_body_check(function_type->prototype_scope_);
	this->check_body_label(current_funtion_symbol);
	iFUNCTION::current_definition->create_basic_block_list();
//	iFUNCTION::current_definition->show_basic_block_list();
}
void FUNCTION_DEFINITION::check_prototype()
{
	struct TYPE_FUNCTION* function_type=static_cast<struct TYPE_FUNCTION*>(declaration_declarator_->type);
	std::vector<struct IDENTIFIER*>* parameter_list;
	struct IDENTIFIER* parameter;
	parameter_list=&function_type->prototype_->list_;
	for(size_t i=0;i<parameter_list->size();i++)
	{
		parameter=parameter_list->at(i);
		if(parameter->type->is_incomplete())
		{
			if(parameter->type->is_void())
				::SCC_ERROR(pos,"%s : illegal use of type 'void'.",parameter->name.empty()?"parameter":parameter->name.c_str());
			else
				::SCC_ERROR(pos,"%s : incomplete struct or union.",parameter->name.empty()?"parameter":parameter->name.c_str());
		}
		if(parameter->name.empty())
			::SCC_ERROR(pos,"expected parameter name.");
	}
}

void FUNCTION_DEFINITION::check()
{
//because the function maybe recursive call itself,need a symbol in scope,must install one before check bodying..
//label information must be install in current current_definition,so must set the current_definition also.
	this->declaration_type_->check(false);
	this->declaration_declarator_->check(declaration_type_->type);
	assert(!declaration_declarator_->identifier.empty());

	struct TYPE* type=declaration_declarator_->type;
	std::string name=declaration_declarator_->identifier;
	std::string linkage=declaration_type_->sclass=="static"?"internal":"external";
	if(declaration_declarator_->type->is_function())
	{	
		this->check_prototype();

		struct IDENTIFIER* symbol_old;
		symbol_old=SCOPE::current->find(name);
		if(symbol_old!=NULL)
		{
			if(symbol_old->kind=="FUNCTION")	
			{
				struct iFUNCTION* casted=static_cast<struct iFUNCTION* >(symbol_old);
				if(casted->linkage=="external" && linkage=="internal")
					::SCC_ERROR(pos,"%s : redefinition; different linkage.",name.c_str());
				if(!casted->type->is_compatible(type))
					::SCC_ERROR(pos,"%s : redefinition; different type.",name.c_str());
				if(casted->status=="definition")
				{
					::SCC_ERROR(pos,"%s : function already has a body.",name.c_str());
					//no need to install symbol before checking body.one with same name has already installed.
					struct iFUNCTION* function=IDENTIFIER::function(type,name,linkage,"definition");
					this->check_body(function);
				}
				else
				{
					this->check_body(casted);
					casted->body=this->body;

					casted->status=="definition";
					casted->type=type;//type of definition override declaration's .
					iFUNCTION::definition_list.push_back(casted);//add to definition_list
				}
			}
			else
				::SCC_ERROR(pos,"%s : redefinition; different type.",name.c_str());
		}else
		{
			struct iFUNCTION* function=IDENTIFIER::function(type,name,linkage,"definition");
			SCOPE::current->insert(name,function);
			this->check_body(function);
			function->body=this->body;
			iFUNCTION::definition_list.push_back(function);
		}

	}else
		::SCC_ERROR(pos,"expecting function here.");
}
void DECLARATION::check()
{
	if(this->list.empty())
	{
		this->declaration_type_->check(true);
	}
	else
	{
		this->declaration_type_->check(false);
		for(size_t i=0;i< this->list.size();i++)
		{
			struct syntax_node* node=list.at(i);
			if(node->op=="INIT_DECLARATOR")
			{
				struct INIT_DECLARATOR* casted=static_cast<struct INIT_DECLARATOR*>(node);
				casted->check(declaration_type_->type,declaration_type_->sclass);
			}
			else
				assert(0);
		}
	}
}

void INIT_DECLARATOR::check(struct TYPE *base_type,std::string sclass)
{
	declaration_declarator_->check(base_type);
	assert(!declaration_declarator_->identifier.empty());
	struct TYPE* type=declaration_declarator_->type;
	std::string name=declaration_declarator_->identifier;	

	//////////////////////
	if(sclass=="typedef")
	{
		if(this->initializer_!=NULL)
		{
			::SCC_ERROR(pos,"%s : can not initialize typedef name.",name.c_str());
		}
		install_typedef_name(type,name);
	}else
	{
		if(SCOPE::current->is_file_scope())
		{
			if(!sclass.empty()&& sclass!="static" && sclass!="extern")
			{
				::SCC_WARNNING(pos,"%s : has bad storage class;ignored.",name.c_str());
				sclass.clear();
			}
			install_file_scope(type,name,sclass);
		}else
		{
			install_block_scope(type,name,sclass);
		}
	}
}
void INIT_DECLARATOR::install_typedef_name(struct TYPE* type,std::string name)
{
	struct IDENTIFIER* scope_symbol_old;
	scope_symbol_old=SCOPE::current->scope_find(name);
	if(scope_symbol_old!=NULL)
	{
		::SCC_ERROR(pos,"%s : redefinition.",name.c_str());
	}else
	{
		struct iTYPEDEF_NAME* for_install=IDENTIFIER::typedef_name(type,name);
		SCOPE::current->insert(name,for_install);
	}
}
void INIT_DECLARATOR::install_block_scope(struct TYPE* type,std::string name,std::string sclass)
{
	if(type->is_function())
	{
		if(!sclass.empty()&& sclass!="static" && sclass!="extern")
		{
			::SCC_WARNNING(pos,"%s : has bad storage class;ignored.",name.c_str());
			sclass.clear();
		}
		if(sclass=="static")
			::SCC_ERROR(pos,"%s : static functions with block scope are illegal.",name.c_str());
		install_block_scope_funtion(type,name);
	}else
	{
		if(!sclass.empty()&& sclass!="static" && sclass!="extern" && sclass!="auto")
		{
			::SCC_WARNNING(pos,"%s : has bad storage class;ignored.",name.c_str());
			sclass.clear();
		}
		if(type->is_incomplete())
		{
			if(type->is_array())
			{
				if(this->initializer_==NULL)
					::SCC_ERROR(pos,"%s : unknown size.",name.c_str());
			}else if(type->is_void())
				::SCC_ERROR(pos,"illegal use of type 'void'.");
			else
				::SCC_ERROR(pos,"incomplete struct or union.");
		}
		if(sclass=="extern" && this->initializer_!=NULL)
			::SCC_ERROR(pos,"%s : initialize extern variables with block scope.",name.c_str());
		if(sclass=="extern")
			install_block_scope_variable_external(type,name);
		else
		{
			std::string storage=sclass=="static"?"static":"automatic";
			install_block_scope_variable_local(type,name,storage);
		}
	}
}
void INIT_DECLARATOR::install_block_scope_variable_local(struct TYPE* type,std::string name,std::string storage)
{
	struct IDENTIFIER* scope_symbol_old;
	scope_symbol_old=SCOPE::current->scope_find(name);
	if(scope_symbol_old!=NULL)
	{
		::SCC_ERROR(pos,"%s : redefinition.",name.c_str());
	}else
	{
		struct iVARIABLE* for_install=IDENTIFIER::variable(type,name,"none",storage,NULL,"none");
		if(storage=="static")
			iFUNCTION::function_static_list.push_back(for_install);
		else
			iFUNCTION::current_definition->insert_local(for_install);
		for_install->initializer=check_initializer(for_install->type,for_install->storage=="static",for_install);
		SCOPE::current->insert(name,for_install);
	}
}
void INIT_DECLARATOR::install_block_scope_variable_external(struct TYPE* type,std::string name)
{
	struct IDENTIFIER* scope_symbol_old;
	scope_symbol_old=SCOPE::current->scope_find(name);
	if(scope_symbol_old!=NULL)
	{
		if(scope_symbol_old->kind=="VARIABLE")	
		{
			struct iVARIABLE* casted=static_cast<struct iVARIABLE* >(scope_symbol_old);
			if(casted->linkage=="none")
				::SCC_ERROR(pos,"%s : redefinition; different linkage.",name.c_str());
			if(!casted->type->is_compatible(type))
				::SCC_ERROR(pos,"%s : redefinition; different type.",name.c_str());
		}
		else
			::SCC_ERROR(pos,"%s : redefinition; different type.",name.c_str());
	}else
	{
		struct IDENTIFIER* symbol_old;
		struct iVARIABLE* for_install;
		symbol_old=SCOPE::file->find(name);
		if(symbol_old!=NULL)
		{
			if(symbol_old->kind=="VARIABLE")	
			{
				for_install=static_cast<struct iVARIABLE* >(symbol_old);
				if(!for_install->type->is_compatible(type))
				{
					::SCC_ERROR(pos,"%s : redefinition; different type.",name.c_str());
					for_install=IDENTIFIER::variable(type,name,"external","static",NULL,"declaration");
					iFUNCTION::function_extern_list.push_back(for_install);
				}
			}else
			{
				if(symbol_old->kind!="TYPEDEF_NAME")
					::SCC_ERROR(pos,"%s : redefinition.",name.c_str());
				for_install=IDENTIFIER::variable(type,name,"external","static",NULL,"declaration");
				iFUNCTION::function_extern_list.push_back(for_install);
			}
		}else
		{
			for_install=IDENTIFIER::variable(type,name,"external","static",NULL,"declaration");
			iFUNCTION::function_extern_list.push_back(for_install);
		}
		SCOPE::current->insert(name,for_install);
	}
}
void INIT_DECLARATOR::install_block_scope_funtion(struct TYPE* type,std::string name)
{
	struct IDENTIFIER* scope_symbol_old;
	scope_symbol_old=SCOPE::current->scope_find(name);
	if(scope_symbol_old!=NULL)
	{
		if(scope_symbol_old->kind=="FUNCTION")	
		{
			struct iFUNCTION* casted=static_cast<struct iFUNCTION* >(scope_symbol_old);
			if(!casted->type->is_compatible(type))
				::SCC_ERROR(pos,"%s : redefinition; different type.",name.c_str());
		}
		else
			::SCC_ERROR(pos,"%s : redefinition; different type.",name.c_str());
	}else
	{
		struct IDENTIFIER* symbol_old;
		struct iFUNCTION* function_for_install;
		symbol_old=SCOPE::file->find(name);
		if(symbol_old!=NULL)
		{
			if(symbol_old->kind=="FUNCTION")
			{
				function_for_install=static_cast<struct iFUNCTION* >(symbol_old);
				if(!function_for_install->type->is_compatible(type))
				{
					::SCC_ERROR(pos,"%s : redefinition; different type.",name.c_str());
					function_for_install=IDENTIFIER::function(type,name,"external","declaration");
					iFUNCTION::function_extern_list.push_back(function_for_install);
				}
			}
			else
			{
				if(symbol_old->kind!="TYPEDEF_NAME")
					::SCC_ERROR(pos,"%s : redefinition.",name.c_str());
				function_for_install=IDENTIFIER::function(type,name,"external","declaration");
				iFUNCTION::function_extern_list.push_back(function_for_install);
			}
		}else
		{
			function_for_install=IDENTIFIER::function(type,name,"external","declaration");
			iFUNCTION::function_extern_list.push_back(function_for_install);
		}
		SCOPE::current->insert(name,function_for_install);
	}
}
void INIT_DECLARATOR::install_file_scope(struct TYPE* type,std::string name,std::string sclass)
{
	std::string linkage=sclass=="static"?"internal":"external";
	if(type->is_function())
	{
		install_file_scope_funtion(type,name,linkage);
	}else
	{
		struct INITIALIZER_DATA* initializer=check_initializer(type,true,NULL);
		if(type->is_incomplete())
		{
			if(type->is_array())
			{
				//ok.
			}else if(type->is_void())
				::SCC_ERROR(pos,"illegal use of type 'void'.");
			else
				::SCC_ERROR(pos,"incomplete struct or union.");
		}
		std::string status="declaration";
		if(sclass!="extern")
			status="tentative";
		if(initializer!=NULL)
			status="definition";
		install_file_scope_variable(type,name,linkage,initializer,status);
	}
}
void INIT_DECLARATOR::install_file_scope_funtion(struct TYPE* type,std::string name,std::string linkage)
{
	struct IDENTIFIER* symbol_old;
	symbol_old=SCOPE::current->find(name);
	if(symbol_old!=NULL)
	{
		if(symbol_old->kind=="FUNCTION")	
		{
			struct iFUNCTION* casted=static_cast<struct iFUNCTION* >(symbol_old);
			if(casted->linkage=="external" && linkage=="internal")
				::SCC_ERROR(pos,"%s : redefinition; different linkage.",name.c_str());
			if(!casted->type->is_compatible(type))
				::SCC_ERROR(pos,"%s : redefinition; different type.",name.c_str());
		}
		else
			::SCC_ERROR(pos,"%s : redefinition; different type.",name.c_str());
	}else
	{
		struct iFUNCTION* function=IDENTIFIER::function(type,name,linkage,"declaration");
		SCOPE::current->insert(name,function);
	}
}
void INIT_DECLARATOR::install_file_scope_variable(struct TYPE* type,std::string name,std::string linkage,
	struct INITIALIZER_DATA* initializer,std::string status)
{
	struct IDENTIFIER* symbol_old;
	symbol_old=SCOPE::current->find(name);
	if(symbol_old!=NULL)
	{
		if(symbol_old->kind=="VARIABLE")	
		{
			struct iVARIABLE* casted=static_cast<struct iVARIABLE* >(symbol_old);
			if(casted->linkage=="external" && linkage=="internal")
				::SCC_ERROR(pos,"%s : redefinition; different linkage.",name.c_str());
			if(!casted->type->is_compatible(type))
				::SCC_ERROR(pos,"%s : redefinition; different type.",name.c_str());
			else
			{
				if(type->is_array() && !type->is_incomplete())
					casted->type=type;//complete array override incomplete array.
			}
			if(status=="definition")
			{
				if(casted->status=="definition")
					::SCC_ERROR(pos,"%s : redefinition; multiple initialization.",name.c_str());
				else
				{
					casted->status=status;
					casted->initializer=initializer;
				}
			}else if(status=="tentative")
			{
				if(status=="declaration")
					casted->status=status;
			}
		}
		else
			::SCC_ERROR(pos,"%s : redefinition; different type.",name.c_str());
	}else
	{
		struct iVARIABLE* variable=IDENTIFIER::variable(type,name,linkage,"static",initializer,status);
		SCOPE::current->insert(name,variable);
	}
}
void DECLARATION_DECLARATOR::check(struct TYPE *base_type)
{

	struct TYPE *new_type;
	std::string tok,extra;
	new_type=base_type;
	for(size_t i=0;i< this->list.size();i++)
	{
		struct syntax_node* node=list.at(i);
		if(node->is_terminal(&tok,&extra))
		{
			if(tok=="ID")
			{
				this->identifier=extra;
			}else if(tok=="cdecl" || tok=="stdcall")
			{
				if(new_type->is_function())
				{
					struct TYPE_FUNCTION* casted=static_cast<struct TYPE_FUNCTION*>(new_type);
					casted->set_calling_convention(tok);
				}else
					::SCC_WARNNING(pos,"%s : applies to non-function types has no meaning; ignored.",tok.c_str());
			}else
				assert(0);
		}else if(node->op=="DECLARATOR_POINTER")
		{
			struct DECLARATOR_POINTER* dp=static_cast<struct DECLARATOR_POINTER*>(node);
			new_type=dp->check(new_type);
		}else if(node->op=="DECLARATOR_ARRAY")
		{
			struct DECLARATOR_ARRAY* da=static_cast<struct DECLARATOR_ARRAY*>(node);
			new_type=da->check(new_type);
		}else if(node->op=="DECLARATOR_FUNCTION")
		{
			struct DECLARATOR_FUNCTION* df=static_cast<struct DECLARATOR_FUNCTION*>(node);
			new_type=df->check(new_type);
		}else
			assert(0);
	}
	this->type=new_type;
}
struct TYPE * DECLARATOR_POINTER::check(struct TYPE *base_type)
{
	struct TYPE *new_type;
	new_type=TYPE::pointer_type(base_type);
	if(!list.empty())
	{
		std::string tok,extra;
		bool see_const=false;
		bool see_volatile=false;
		for(size_t i=0;i<list.size();i++)
		{
			struct syntax_node* node=list.at(i);
			if(node->is_terminal(&tok,&extra))
			{
				if(tok=="const")
					see_const=true;
				else if(tok=="volatile")
					see_volatile=true;
				else
					assert(0);
			}
			else
				assert(0);
		}
		new_type=new_type->qualify(see_const,see_volatile);
	}
	return new_type;
}

struct TYPE * DECLARATOR_ARRAY::check(struct TYPE *base_type)
{
	if(base_type->is_incomplete())
		::SCC_ERROR(pos,"element of array have incomplete type.");
	if(base_type->is_function())
		::SCC_ERROR(pos,"element of array have function type.");
	size_t number_of_element;
	if(this->number_of_element!=NULL)
	{
		struct CONST* con=CONST::constant_expression_2_const(static_cast<struct exp*>(this->number_of_element));
		if(con!=NULL)
		{
			if(con->type->is_integral_type())
				number_of_element=con->as_uint32();
			else
				::SCC_ERROR(pos,"constant expression is not integral.");
		}else
			::SCC_ERROR(pos,"expected constant expression.");
	}else
		number_of_element=0;
	return TYPE::array_type(base_type,number_of_element);
}
void PARAMETER_DECLARATION::check(bool is_first_parameter)
{
	struct TYPE* type;
	std::string name;
	std::string sclass;
	this->declaration_type_->check(false);
	type=this->declaration_type_->type;
	if(this->declaration_declarator_!=NULL)
	{
		this->declaration_declarator_->check(type);
		type=declaration_declarator_->type;
		name=declaration_declarator_->identifier;
	}
	type=this->adjust(type);
	sclass=this->declaration_type_->sclass;
	if(!sclass.empty()&& sclass!="register")
	{
		::SCC_WARNNING(pos,"%s : has bad storage class;ignored.",name.empty()?"parameter":name.c_str());
		sclass.clear();
	}
	if(is_first_parameter && type->is_void())
	{
		//do nothing!
	}else
	{
		if(type->is_void())
			::SCC_ERROR(pos,"illegal use of type 'void'.");
		this->install_paramter(type,name);
	}
}
void PARAMETER_DECLARATION::install_paramter(struct TYPE* type,std::string name)
{
	if(!name.empty())
	{
		struct IDENTIFIER* scope_symbol_old;
		scope_symbol_old=SCOPE::current->scope_find(name);
		if(scope_symbol_old!=NULL)
		{
			::SCC_ERROR(pos,"parameter :  %s :redefinition.",name.c_str());
		}else
		{
			struct iVARIABLE* for_install=IDENTIFIER::parameter(type,name);
			SCOPE::current->insert(name,for_install);
		}
	}else
	{
		struct iVARIABLE* for_install=IDENTIFIER::parameter(type,name);
		SCOPE::current->insert(name,for_install);
	}
}
struct TYPE* PARAMETER_DECLARATION::adjust(struct TYPE* type)
{
	if(type->is_array())
	{
		struct ARRAY_TYPE* array_type=static_cast<struct ARRAY_TYPE*>(type);
		return TYPE::pointer_type(array_type->type_);
	}else if(type->is_function())
	{
		return TYPE::pointer_type(type);
	}else
		return type;
}
struct TYPE * DECLARATOR_FUNCTION::check(struct TYPE *base_type)
{
	if(base_type->is_void() || (base_type->is_object_type() && !base_type->is_array()))
	{
		//ok!
	}
	else
	{
		if(base_type->is_array())
			::SCC_ERROR(pos,"function return array.");
		if(base_type->is_function())
			::SCC_ERROR(pos,"function return function.");
	}
	bool is_variadic=false;
	struct SCOPE* prototype_scope;
	SCOPE::current->enter_scope();
	prototype_scope=SCOPE::current;
	{
	//check_paramenter     this->list
		struct PARAMETER_DECLARATION* parameter;
		struct syntax_node *node;
		std::string tok,extra;
		for(size_t i=0;i<this->list.size();i++)
		{
			node=this->list.at(i);
			if(node->is_terminal(&tok,&extra))
			{
				assert(tok=="ELLIPSE");
				is_variadic=true;
			}else
			{
				parameter=static_cast<struct PARAMETER_DECLARATION*>(node);
				parameter->check(i==0);
			}
		}
	}
	SCOPE::current->exit_scope();
	if(base_type->is_volatile() || base_type->is_const())
	{
		::SCC_WARNNING(pos,"qualifier applied to function type has no meaning; ignored.");
		base_type=base_type->unqualify();
	}
	return TYPE::type_function(base_type,prototype_scope,is_variadic);
}
struct TYPE * TYPE_NAME_DECLARATION::get_type()
{
	struct TYPE* type;
	this->declaration_type_->check(false);
	type=this->declaration_type_->type;
	if(this->declaration_declarator_!=NULL)
	{
		this->declaration_declarator_->check(type);
		type=declaration_declarator_->type;
	}
	return type;
}