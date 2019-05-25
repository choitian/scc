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

#include "syntax_tree.h"

struct INIT_DECLARATOR:public syntax_node
{
	struct DECLARATION_DECLARATOR* declaration_declarator_;
	struct syntax_node* initializer_;
	INIT_DECLARATOR(){ op="INIT_DECLARATOR"; };
//semantic
	void check(struct TYPE *base_type,std::string sclass);
private:
	struct INITIALIZER_DATA* check_initializer(struct TYPE* type,bool is_static,struct iVARIABLE* variable);



	void install_file_scope(struct TYPE* type,std::string name,std::string sclass);
	void install_file_scope_funtion(struct TYPE* type,std::string name,std::string linkage);
	void install_file_scope_variable(struct TYPE* type,std::string name,std::string linkage,
		struct INITIALIZER_DATA* initializer,std::string status);

	void install_block_scope(struct TYPE* type,std::string name,std::string sclass);
	void install_block_scope_funtion(struct TYPE* type,std::string name);
	void install_block_scope_variable_local(struct TYPE* type,std::string name,std::string storage);
	void install_block_scope_variable_external(struct TYPE* type,std::string name);

	void install_typedef_name(struct TYPE* type,std::string name);
};
struct INITIALIZER:public syntax_node
{
	struct syntax_node* value;
	std::vector<struct syntax_node* > list;
	bool value_checked;
	struct ADDRESS* v_addr;
//semantic
	void check();
	INITIALIZER()
	{ 
		op="INITIALIZER"; 
		value_checked=false;
		v_addr=NULL;
	};
};

struct DECLARATOR_POINTER:public syntax_node
{
	std::vector<struct syntax_node* > list;
	DECLARATOR_POINTER(){ op="DECLARATOR_POINTER"; };
//semantic
	struct TYPE * check(struct TYPE *base_type);
};
struct DECLARATOR_ARRAY:public syntax_node
{
	struct syntax_node* number_of_element;
	DECLARATOR_ARRAY(){ op="DECLARATOR_ARRAY"; };
//semantic
	struct TYPE * check(struct TYPE *base_type);
};


struct PARAMETER_DECLARATION:public syntax_node
{
	struct DECLARATION_TYPE* declaration_type_;
	struct DECLARATION_DECLARATOR* declaration_declarator_;
	PARAMETER_DECLARATION(){ op="PARAMETER_DECLARATION"; };
//semantic
	void check(bool is_first_parameter);
	void install_paramter(struct TYPE* type,std::string name);
	struct TYPE* adjust(struct TYPE* type);
};
struct DECLARATOR_FUNCTION:public syntax_node
{
	std::vector<struct syntax_node* > list;
	DECLARATOR_FUNCTION(){ op="DECLARATOR_FUNCTION"; };
//semantic
	struct TYPE * check(struct TYPE *base_type);
};
struct DECLARATION_TYPE:public syntax_node
{
	std::vector<struct syntax_node* > list;
	DECLARATION_TYPE(){ op="DECLARATION_TYPE"; };
//semantic
	struct TYPE * typedef_name_type(std::string typedef_name);
	void check(bool need_new_tag);
	std::string sclass;
	struct TYPE *type;
};
struct DECLARATION_DECLARATOR:public syntax_node
{
	std::vector<struct syntax_node* > list;
	DECLARATION_DECLARATOR(){ op="DECLARATION_DECLARATOR"; };
//semantic
	void check(struct TYPE *base_type);
	std::string identifier;
	struct TYPE *type;
};

struct FUNCTION_DEFINITION:public syntax_node
{
	struct DECLARATION_TYPE* declaration_type_;
	struct DECLARATION_DECLARATOR* declaration_declarator_;
	struct syntax_node* body;
	FUNCTION_DEFINITION(){ op="FUNCTION_DEFINITION"; }
//semantic
	void check();
	void check_prototype();
	void check_body(struct iFUNCTION* current_funtion_symbol);
	void check_body_label(struct iFUNCTION* funtion_symbol);
};
struct DECLARATION:public syntax_node
{
	struct DECLARATION_TYPE* declaration_type_;
	std::vector<struct syntax_node* > list;
	DECLARATION(){ op="DECLARATION"; };
//semantic
	void check();
};

struct TYPE_STRUCT:public syntax_node
{
	std::string name;
	std::vector<struct syntax_node* > list;
	TYPE_STRUCT(){ op="TYPE_STRUCT"; }
//semantic
	void check(bool need_new_tag);
	void check_definition();
	void check_declaration(bool need_new_tag);
	struct TYPE* type;
};
struct TYPE_UNION:public syntax_node
{
	std::string name;
	std::vector<struct syntax_node* > list;
	TYPE_UNION(){ op="TYPE_UNION"; }
//semantic
	void check(bool need_new_tag);
	void check_definition();
	void check_declaration(bool need_new_tag);
	struct TYPE* type;
};
struct STRUCT_DECLARATION:public syntax_node
{
	struct DECLARATION_TYPE* declaration_type_;
	std::vector<struct syntax_node* > list;
	STRUCT_DECLARATION(){ op="STRUCT_DECLARATION"; }
//semantic
	void check();
};
struct STRUCT_DECLARATOR:public syntax_node
{
	struct DECLARATION_DECLARATOR* declaration_declarator_;
	struct syntax_node* width;
	STRUCT_DECLARATOR(){ op="STRUCT_DECLARATOR"; }
//semantic
	void check(struct TYPE *base_type);
	void install_member(struct TYPE *type,std::string name);
	void install_member_bit_field(struct TYPE *type,std::string name,size_t width);
};

struct TYPE_ENUM:public syntax_node
{
	std::string name;
	std::vector<struct syntax_node* > list;
	TYPE_ENUM(){ op="TYPE_ENUM"; }
//semantic
	void check();
	void check_definition();
	void check_declaration();
	struct TYPE* type;
};
struct ENUMERATOR:public syntax_node
{
	std::string name;
	struct syntax_node* initializer;
	ENUMERATOR(){ op="ENUMERATOR"; }
};

struct TYPE_NAME_DECLARATION:public syntax_node
{
	struct DECLARATION_TYPE* declaration_type_;
	struct DECLARATION_DECLARATOR* declaration_declarator_;
	TYPE_NAME_DECLARATION(){ op="TYPE_NAME_DECLARATION"; };
//semantic
	struct TYPE * get_type();
};
struct TRANSLATION_UNIT:public syntax_node
{
	std::vector<struct syntax_node* > list;
	TRANSLATION_UNIT(){ op="TRANSLATION_UNIT"; };
//semantic
	void check();
	std::vector<struct FUNCTION_DEFINITION* > function_definition_list;
};



