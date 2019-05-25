#pragma once
#define _CRT_SECURE_NO_WARNINGS
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

#include "..\\lib\\common.h"

#define __NULL__ "__NULL__"
#define __START__ "__START__"
#define __END__ "__END__"

struct Symbol;
//empty symbol name
struct Production
{

	int id;
	struct Symbol * head;
	std::vector<struct Symbol *> node_list;
	std::string srcipt_action;

	void AddNode(struct Symbol* node);
	int length();
//testing
	bool IsNullProduction();
//////////////////////////////////////////////////////
	static struct Production* new_()
	{
		static int id_counter=1;
		struct Production* p=new struct Production();
		p->id=id_counter++;
		return p;
	}
/////////////////////////////////////////////////////
//DEBUG
	std::string toString();
private:
	Production()
	{
	}
};
struct Symbol
{
	int id;
	std::string name;

	bool FST_nullable;
	std::set<int > FST;
	bool can_NULL;
	class Script* SCRIPT;
	std::vector<struct Production*> production_list;
	void AddProduction(struct Production* production);

	bool IsTerminal();
	bool IsNullSymbol();//directly empty.
	bool IsEmptySymbol();//recursively empty.
	bool IsEndmarkerSymbol();
//////////////////////////////////////////////////////
	static struct Symbol* new_(std::string symbol_name)
	{
		static int id_counter=1;
		struct Symbol* s=new struct Symbol(id_counter++,symbol_name);
		return s;
	}
/////////////////////////////////////////////////////
//DEBUG
	std::string toString();
private:
	Symbol(int symbol_id,std::string symbol_name)
	{
		id=symbol_id;
		name=symbol_name;
		FST_nullable=false;
		can_NULL=false;
	}
};
class Script
{
	char* cur_,*base_;
	int line_;
	std::string current_,extra_;

	std::map<int,struct Symbol*>  id_to_symbol_;
	std::map<std::string,struct Symbol*>  name_to_symbol_;
	std::map<int,struct Production*>  id_to_Production_;
	std::string start_symbol_;

	void load_lalr_script(std::string script);
	void one_symbol();
	struct Production* one_production();
	struct Symbol* one_production_node();
	void next_token();
	std::string get_next_token();
	void expect(std::string token);
	void pass(std::string token);
	struct Symbol* get_or_new(std::string symbol_name);

	bool FST_changed_;
	void compute_FST();
	void compute_FST_nonterminal(struct Symbol* symbol);
	void compute_FST_nonterminal_by_production(struct Symbol* head,struct Production* production);
public:
	std::vector<struct Symbol*>  symbol_list_;
	std::vector<struct Production*>  production_list_;

	std::set<int>* FST_suffix_LR(std::vector<struct Symbol*>::iterator start,
										std::vector<struct Symbol*>::iterator end,struct Symbol* lookahead);
	struct Symbol* StartSymbol();
	struct Symbol* NullSymbol();
	struct Symbol* EndmarkerSymbol();
	struct Symbol* LookupId(int symbol_id);
	struct Symbol* LookupName(std::string symbol_name);
	struct Production* LookupProduction(int production_id);
	void _DEBUG_();
	static void TEST();
	Script(std::string script);
	Script(){}
};

struct Action
{
	int type;
	struct ItemSet* shift_to_collection;
	struct Production* reduce_by_production;
	struct ItemSet* go_to_collection;
	enum{
		SHIFT,REDUCE,GOTO,ACCEPT,
	};
};
struct Item
{
	struct Production* production;
	int dot;
	std::string key;
	bool end_with_dot;
	bool start_with_dot;

	bool propagated_and_spontaneous_table_has_build;
	std::vector<struct Item*> propagated_list;//propagated dont need lookahead.
	std::vector<struct Item_LR*> spontaneous_list;

	std::vector<struct Symbol *>::iterator EndOfProductionNodeList()
	{
		return production->node_list.end();
	}
	std::vector<struct Symbol *>::iterator DotRightIter()
	{
		std::vector<struct Symbol *>::iterator it=production->node_list.begin();
		it+=dot;
		return it;
	}
	struct Symbol* DotRight()
	{
		if(end_with_dot)
		{
			DEBUG("illegal DotRight\n");
			return NULL;
		}
		return production->node_list[dot];
	}
	static struct Item* new_(struct Production* production,int dot,std::string key)
	{
		if(dot > production->length())
		{
			DEBUG("illegal Item dot(%d)\n",dot);
			return NULL;
		}
		struct Item* it=new struct Item(production,dot,key);
		if(dot==production->length())
			it->end_with_dot=true;
		if(dot==0)
			it->start_with_dot=true;
		return it;
	}
	static std::string MakeKey(struct Production* production,int dot)
	{
		return int_int_to_string(production->id,dot);
	}
	std::string toString()
	{
		std::string text;
		text.append(production->toString());
		text.append(":");
		text.append(int_to_string(dot));
		return text;
	}
private:
	Item(struct Production* production,int dot,std::string key)
	{
		this->production=production;
		this->dot=dot;
		this->key=key;
		start_with_dot=false;
		end_with_dot=false;
		propagated_and_spontaneous_table_has_build=false;
	}
};
struct ItemSet
{
	int id;
	std::string key;
	std::map<std::string,struct Item*> kernel_items;
	std::map<std::string,struct Item*> items;

	std::map<struct Symbol*,struct ItemSet*> GOTO;
	std::map<std::string,std::vector<struct ItemSet*> > propagated_target_collections_of_item;
	std::map<struct Symbol*,struct Action> parsing_table;

	struct ItemSet_LR* extension_LR;
	bool AddKernelItem(struct Item* item)
	{
		if(kernel_items.count(item->key)==0)
		{
			kernel_items[item->key]=item;
			items[item->key]=item;
			return true;
		}
		return false;
	}
	bool AddItem(struct Item* item)
	{
		if(items.count(item->key)==0)
		{
			items[item->key]=item;
			return true;
		}
		return false;
	}
	void CalculateKey()
	{
		for(std::map<std::string,struct Item*>::iterator it=kernel_items.begin();it!=kernel_items.end();it++)
		{
			key.append(it->first);
		}
	}

	static struct ItemSet* new_()
	{
		return new struct ItemSet();
	}
	std::string toString()
	{
		std::string text;
		for(std::map<std::string,struct Item*>::iterator it=kernel_items.begin();it!=kernel_items.end();it++)
		{
			text.append(it->second->toString());
			text.append("\n");
		}
		return text;
	}
private:
	ItemSet()
	{

	}
};
struct Item_LR
{
	struct Item* base;
	struct Symbol* lookahead;
	std::string key;

	struct ItemSet* containing_collection;
	static struct Item_LR* new_(struct Item* base,struct Symbol* lookahead)
	{
		struct Item_LR* lr=new struct Item_LR(base,lookahead,MakeKey(base,lookahead));
		return lr;
	}

	static std::string MakeKey(struct Item* base,struct Symbol* lookahead)
	{
		std::string key;
		key.append(base->key);
		key.append(int_to_string(lookahead->id));
		return key;
	}
	std::string toString()
	{
		std::string text;
		text.append(base->toString());
		text.append(":::");
		text.append(lookahead->name);
		return text;
	}
private:
	Item_LR(struct Item* base,struct Symbol* lookahead,std::string key)
	{
		this->base=base;
		this->lookahead=lookahead;
		this->key=key;
	}
};
struct ItemSet_LR
{
	struct ItemSet* base;
	std::string key;
	std::map<std::string,struct Item_LR*> kernel_items_LR;
	std::map<std::string,struct Item_LR*> items_LR;

	bool AddKernelItem(struct Item_LR* item_LR)
	{
		if(kernel_items_LR.count(item_LR->key)==0)
		{
			kernel_items_LR[item_LR->key]=item_LR;
			items_LR[item_LR->key]=item_LR;
			return true;
		}
		return false;
	}
	bool AddItem(struct Item_LR* item_LR)
	{
		if(items_LR.count(item_LR->key)==0)
		{
			items_LR[item_LR->key]=item_LR;
			return true;
		}
		return false;
	}
	void CalculateKey()
	{
		for(std::map<std::string,struct Item_LR*>::iterator it=kernel_items_LR.begin();it!=kernel_items_LR.end();it++)
		{
			key.append(it->first);
		}
	}

	static struct ItemSet_LR* new_()
	{
		return new struct ItemSet_LR();
	}
	std::string toString()
	{
		std::string text;
		for(std::map<std::string,struct Item_LR*>::iterator it=kernel_items_LR.begin();it!=kernel_items_LR.end();it++)
		{
			text.append(it->second->toString());
			text.append("\n");
		}
		return text;
	}
private:
	ItemSet_LR()
	{
		base=NULL;
	}
};
struct CanonicalCollection
{
	std::map<std::string,struct ItemSet*> key_to_collection;
	std::vector<struct ItemSet*> collection_list;

	bool AddCollection(struct ItemSet* collection)
	{
		static int id_counter=1;

		if(key_to_collection.count(collection->key)==0)
		{
			//create shadow LR Set.
			collection->extension_LR=ItemSet_LR::new_();
			collection->extension_LR->base=collection;

			collection->id=id_counter++;
			key_to_collection[collection->key]=collection;
			collection_list.push_back(collection);
			return true;
		}
		return false;
	}
	struct ItemSet* Lookup(std::string key)
	{
		if(key_to_collection.count(key)==0)
		{
			return NULL;
		}
		return key_to_collection[key];
	}
	std::string toString()
	{
		std::string text;
		text.append("Collection:");
		text.append(::int_to_string(key_to_collection.size()));
		text.append("\n\n\n");
		for(std::vector<struct ItemSet*>::iterator it=collection_list.begin();it!=collection_list.end();it++)
		{
			text.append((*it)->extension_LR->toString());
			text.append("\n**********************************************************************\n");

		}
		return text;
	}
};
class LALR_Generator
{
	struct Symbol* start_;
	struct Production* start_production_;
	class Script* script_;
	struct Item* initialItem_,*acceptItem_;
	struct ItemSet* initial_Set_;
	struct CanonicalCollection CC_;
	std::map<std::string,struct Item*> key_toItem_;
	std::stack<struct Item_LR* > unpropagated_stack;

	void Augment();
	struct Item* get_or_new(struct Production* production,int dot);
	void closure(struct ItemSet* itemset);
	std::map<struct Symbol*,struct ItemSet*> *build_goto_catalogue(struct ItemSet* itemset);
	void build_canonical_collection();

	void closure_LR(struct ItemSet_LR* itemset_LR);
	void build_propagated_and_spontaneous_table_of_item(struct Item* item);
	void discovering_propagated_and_spontaneous_lookaheads_by_item(struct ItemSet* collection,struct Item* item);
	void discovering_propagated_and_spontaneous_lookaheads();

	void do_propagation();
	void previous_closure_LR_building();

	void construct_parsing_table();
	void construct_parsing_table_by_collection(struct ItemSet* collection);
	void construct_parsing_table_by_collection_item(struct ItemSet* collection,struct Item_LR* item_LR);
	void out_put_parsing_table(std::string to_file);
public:
	void _DEBUG_();
	static void TEST();
};