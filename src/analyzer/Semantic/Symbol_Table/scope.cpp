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

#include"scope.h"
#include "identifier.h"
struct SCOPE_AAA:public SCOPE
{
	virtual std::vector<struct IDENTIFIER*>* symbol_list();
	virtual struct IDENTIFIER* find(std::string name);
	virtual struct IDENTIFIER* scope_find(std::string name);
	virtual void insert(std::string name,struct IDENTIFIER* identifier);
	virtual void clone_symbol_list(std::vector<struct IDENTIFIER*> *list);

	virtual struct TYPE* find_tag(std::string name);
	virtual struct TYPE* scope_find_tag(std::string name);
	virtual void insert_tag(std::string name,struct TYPE* tag);

	virtual void enter_scope();
	virtual void as_current();
	virtual void exit_scope();

	virtual bool is_file_scope();
	SCOPE_AAA(struct SCOPE* outer)
	{
		outer_=outer;
	}
private:
	struct SCOPE* outer_;
	std::vector<struct IDENTIFIER*> symbol_list_;
	std::map<std::string,struct IDENTIFIER*> symbol_map_;

	std::vector<struct TYPE*> tag_list_;
	std::map<std::string,struct TYPE*> tag_map_;
};

std::vector<struct IDENTIFIER*>* SCOPE_AAA::symbol_list()
{
	return &symbol_list_;
}
struct IDENTIFIER* SCOPE_AAA::find(std::string name)
{
	assert(!name.empty());
	struct IDENTIFIER* in_scope;
	in_scope=this->scope_find(name);
	if(in_scope==NULL)
	{
		if(this->outer_==NULL)
			return NULL;
		else
			return this->outer_->find(name);
	}else
		return in_scope;
}
struct IDENTIFIER* SCOPE_AAA::scope_find(std::string name)
{
	assert(!name.empty());
	if(this->symbol_map_.find(name)!=symbol_map_.end())
	{
		return symbol_map_[name];
	}else
		return NULL;
}
void SCOPE_AAA::insert(std::string name,struct IDENTIFIER* identifier)
{
	if(!name.empty())
	{
		std::map<std::string,struct IDENTIFIER*>::iterator old=this->symbol_map_.find(name);
		if(old!=symbol_map_.end() && old->second->kind!="TYPEDEF_NAME")
			assert(0);
		else
		{
			symbol_list_.push_back(identifier);
			symbol_map_.insert(std::pair<std::string,struct IDENTIFIER*>(name,identifier));
		}
	}else
	{
		symbol_list_.push_back(identifier);
	}
}
void SCOPE_AAA::clone_symbol_list(std::vector<struct IDENTIFIER*> *list)
{
	list->resize(this->symbol_list_.size());
	std::copy(this->symbol_list_.begin(),this->symbol_list_.end(),list->begin());
}
struct TYPE* SCOPE_AAA::find_tag(std::string name)
{
	assert(!name.empty());
	struct TYPE* in_scope;
	in_scope=this->scope_find_tag(name);
	if(in_scope==NULL)
	{
		if(this->outer_==NULL)
			return NULL;
		else
			return this->outer_->find_tag(name);
	}else
		return in_scope;
}
struct TYPE* SCOPE_AAA::scope_find_tag(std::string name)
{
	assert(!name.empty());
	if(this->tag_map_.find(name)!=tag_map_.end())
	{
		return tag_map_[name];
	}else
		return NULL;
}
void SCOPE_AAA::insert_tag(std::string name,struct TYPE* tag)
{

	if(!name.empty())
	{
		if(this->tag_map_.find(name)!=tag_map_.end())
			assert(0);
		else
		{
			this->tag_list_.push_back(tag);
			tag_map_.insert(std::pair<std::string,struct TYPE*>(name,tag));
		}
	}else
	{
		tag_list_.push_back(tag);
	}
}
void SCOPE_AAA::enter_scope()
{
	SCOPE::current=new struct SCOPE_AAA(this);
}
void SCOPE_AAA::as_current()
{
	this->outer_=SCOPE::current;
	SCOPE::current=this;
}
void SCOPE_AAA::exit_scope()
{
	assert(outer_!=NULL);
	SCOPE::current=outer_!=NULL?this->outer_:this;
}

bool SCOPE_AAA::is_file_scope()
{
	return this->outer_==NULL;
}
struct SCOPE* SCOPE::file=NULL;
struct SCOPE* SCOPE::current=NULL;
void SCOPE::start_file_scope()
{
	SCOPE::file=new struct SCOPE_AAA(NULL);
	SCOPE::current=SCOPE::file;
}

