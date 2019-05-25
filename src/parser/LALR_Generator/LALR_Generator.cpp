#include "LALR_Generator.h"

namespace
{
struct EMPTY_PRODUCTION_LIST_CHECKER
{
	EMPTY_PRODUCTION_LIST_CHECKER(bool *is_empty_production_list)
	{
		this->is_empty_production_list=is_empty_production_list;
		*this->is_empty_production_list=true;
	}
	void operator()(struct Production* prod)
	{
		if(*is_empty_production_list)
		{
			for(size_t i=0;i<prod->node_list.size();i++)
			{
				struct Symbol* node=prod->node_list.at(i);
				if(!node->IsEmptySymbol())
				{
					*is_empty_production_list=false;
					break;
				}
			}
		}
	}
private:
	bool *is_empty_production_list;
};
}
void Production::AddNode(struct Symbol* node)
{
	node_list.push_back(node);
}
int  Production::length()
{
	if(node_list.size() == 1 && node_list[0]->IsNullSymbol())
		return 0;
	return node_list.size();
}
bool Production::IsNullProduction()
{
	if(length()==0)
		return true;
	return false;
}
std::string Production::toString()
{
	std::string text;
	text.append(head->name);
	text.append("--->");
	for(size_t i=0;i<node_list.size();i++)
	{
		text.append(" ");
		text.append(node_list.at(i)->name);
	}
	return text;
}



void Symbol::AddProduction(struct Production* production)
{
	if(production->IsNullProduction())
		can_NULL=true;

	production->head=this;
	production_list.push_back(production);
}
bool Symbol::IsNullSymbol()
{
	return name==__NULL__;
}
bool Symbol::IsEmptySymbol()
{
	if(IsNullSymbol())
	{
		return true;
	}else if(IsTerminal())
	{
		return false;
	}else
	{
		bool is_empty_production_list;
		std::for_each(production_list.begin(),production_list.end(),EMPTY_PRODUCTION_LIST_CHECKER(&is_empty_production_list));
		return is_empty_production_list;
	}
}
bool Symbol::IsEndmarkerSymbol()
{
	return name==__END__;
}
bool Symbol::IsTerminal()
{
	return this->production_list.size()==0;
}
std::string Symbol::toString()
{
	std::string text;
	text.append(name);
	if(this->can_NULL)
		text.append(":can_NULL");
	text.append("\n");
	if(this->FST_nullable)
		text.append("[nullable]");
	text.append("[ ");
	for(std::set<int>::iterator it=FST.begin();it!=FST.end();it++)
	{
		text.append( this->SCRIPT->LookupId(*it)->name);
		text.append("  ");
	}
	text.append("]");
	text.append("\n\n");
	for(size_t i=0;i<this->production_list.size();i++)
	{
		text.append(production_list[i]->toString());
		text.append("\n");
	}
	text.append("\n\n");
	return text;
}



void Script::load_lalr_script(std::string script_file)
{
	base_=file_to_char_array(script_file,NULL);
	cur_=base_;
	line_=1;

	next_token();
	if(current_=="ID")
	{
		start_symbol_=extra_;
		one_symbol();
	}
	while(current_=="ID")
	{
		one_symbol();
	}
	//create __NULL__,in case it hasnt been created.
	get_or_new(__NULL__);
	//create __ENDMARKER__
	get_or_new(__END__);
}
void Script::one_symbol()
{
	std::string name=extra_;
	struct Symbol *symbol=get_or_new(name);
	struct Production* production;
	next_token();
	pass("COLON");
	expect("ID");
	production=one_production();
	symbol->AddProduction(production);
	id_to_Production_[production->id]=production;
	production_list_.push_back(production);
	while(current_=="BAR")
	{
		pass("BAR");
		expect("ID");
		production=one_production();
		symbol->AddProduction(production);
		id_to_Production_[production->id]=production;
		production_list_.push_back(production);
	}
	pass("SEMICOLON");
}
struct Production* Script::one_production()
{
	struct Production *production;;
	struct Symbol* node;

	production=Production::new_();
	node=one_production_node();
	production->AddNode(node);
	while(current_=="ID")
	{
		node=one_production_node();
		production->AddNode(node);
	}
	if(current_=="SRCIPT")
	{
		production->srcipt_action=extra_;
		next_token();
	}
	return production;
}
struct Symbol* Script::one_production_node()
{
	std::string name=extra_;
	struct Symbol *node=get_or_new(name);
	next_token();
	return node;
}
void Script::next_token()
{
	this->current_=get_next_token();
}
std::string Script::get_next_token()
{
	std::string token;

	extra_.clear();
	switch(cur_[0])
	{
	case ':':
		token="COLON";
		cur_++;
		break;
	case '|':
		token="BAR";
		cur_++;
		break;
	case ';':
		token="SEMICOLON";
		cur_++;
		break;
	default:
		if(IsLetter(cur_[0]))
		{
			while(IsLetter(cur_[0]) || IsDigit(cur_[0]))
			{
				extra_.append(1,cur_[0]);
				cur_++;
			}
			token="ID";
		}else if(cur_[0]=='{')
		{
			while(cur_[0]!='}')
			{
				extra_.append(1,cur_[0]);
				cur_++;
			}
			extra_.append(1,cur_[0]);
			cur_++;
			token="SRCIPT";
		}
		else if(cur_[0]==EOF)
		{
			token="EOF";
		}else
		{
			if(cur_[0]=='/' && cur_[1]=='/')
			{
				while(cur_[0]!='\n' &&cur_[0]!=EOF)
				{
					cur_++;
				}
			}
			if(cur_[0]=='\n')
				line_++;
			cur_++;
			return get_next_token();
		}
	}
	return token;
}
void Script::expect(std::string token)
{
	if(current_!=token)
	{
		DEBUG("(Line: %d)expect %s\n",this->line_,token.c_str());
	}
}
void Script::pass(std::string token)
{
	expect(token);
	next_token();
}
struct Symbol* Script::get_or_new(std::string symbol_name)
{
	struct Symbol* s;
	if(name_to_symbol_.count(symbol_name)==0)
	{
		s=Symbol::new_(symbol_name);

		symbol_list_.push_back(s);
		name_to_symbol_[symbol_name]=s;
		id_to_symbol_[s->id]=s;
		s->SCRIPT=this;
	}else
	{
		s=name_to_symbol_[symbol_name];
	}
	return s;
}
void Script::compute_FST()
{
	for(std::vector<struct Symbol*>::iterator it=symbol_list_.begin();it!=symbol_list_.end();it++)
	{
		if((*it)->IsTerminal())
		{
			(*it)->FST.insert((*it)->id);
		}
	}
	FST_changed_=true;
	while(FST_changed_)
	{
		FST_changed_=false;
		for(std::vector<struct Symbol*>::iterator it=symbol_list_.begin();it!=symbol_list_.end();it++)
		{
			if(!(*it)->IsTerminal())
			{
				compute_FST_nonterminal(*it);
			}
		}
	}
}
void Script::compute_FST_nonterminal(struct Symbol* symbol)
{
	size_t old_symbol_FST_size;
	old_symbol_FST_size=symbol->FST.size();
	for(std::vector<struct Production*>::iterator it=symbol->production_list.begin();it!=symbol->production_list.end();it++)
	{
		if((*it)->IsNullProduction())
		{
			if(!symbol->FST_nullable)
			{
				FST_changed_=true;
				symbol->FST_nullable=true;
			}
		}else
		{
			compute_FST_nonterminal_by_production(symbol,*it);
		}
	}
	if(symbol->FST.size() > old_symbol_FST_size)
		FST_changed_=true;
}
void Script::compute_FST_nonterminal_by_production(struct Symbol* head,struct Production* production)
{
	for(std::vector<struct Symbol*>::iterator it=production->node_list.begin();;it++)
	{
		if(it==production->node_list.end())
		{
			if(!head->FST_nullable)
			{
				FST_changed_=true;
				head->FST_nullable=true;
			}
			break;
		}
		//union this FST of node to FST of head.
		head->FST.insert((*it)->FST.begin(),(*it)->FST.end());
		if(!(*it)->FST_nullable)
			break;
	}
}
//IO
std::set<int>* Script::FST_suffix_LR(std::vector<struct Symbol*>::iterator start,
										std::vector<struct Symbol*>::iterator end,struct Symbol* lookahead)
{
	static std::set<int> FST_set;
	FST_set.clear();
	for(std::vector<struct Symbol*>::iterator it=start;;it++)
	{
		if(it==end)
		{//plus the lookahead.
			FST_set.insert(lookahead->id);
			break;
		}
		//union this FST of node to FST of head.
		FST_set.insert((*it)->FST.begin(),(*it)->FST.end());
		if(!(*it)->FST_nullable)
			break;
	}
	return &FST_set;
}
struct Symbol* Script::StartSymbol()
{
	return LookupName(this->start_symbol_);
}
struct Symbol* Script::NullSymbol()
{
	return LookupName(__NULL__);
}
struct Symbol* Script::EndmarkerSymbol()
{
	return LookupName(__END__);
}
struct Symbol* Script::LookupId(int symbol_id)
{
	if(id_to_symbol_.count(symbol_id)==0)
		return NULL;
	return id_to_symbol_[symbol_id];
}
struct Symbol* Script::LookupName(std::string symbol_name)
{
	if(name_to_symbol_.count(symbol_name)==0)
		return NULL;
	return name_to_symbol_[symbol_name];
}
struct Production* Script::LookupProduction(int production_id)
{
	if(id_to_Production_.count(production_id)==0)
		return NULL;
	return id_to_Production_[production_id];
}
//DEBUG
void Script::_DEBUG_()
{
	for(std::vector<struct Symbol*>::iterator it=symbol_list_.begin();it!=symbol_list_.end();it++)
	{
		printf((*it)->toString().c_str());
	}
}
void Script::TEST()
{
	class Script *sc=new class Script();

	sc->load_lalr_script("..\\script\\C_GM.c");
	sc->compute_FST();
	sc->_DEBUG_();
}
Script::Script(std::string script)
{
	load_lalr_script(script);
	compute_FST();
}


void LALR_Generator::Augment()
{
	this->start_production_=Production::new_();
	this->start_production_->AddNode(script_->StartSymbol());

	this->start_=Symbol::new_(__START__);
	this->start_->AddProduction(this->start_production_);
}
struct Item* LALR_Generator::get_or_new(struct Production* production,int dot)
{
	struct Item* it;
	std::string key=Item::MakeKey(production,dot);
	if(key_toItem_.count(key)==0)
	{
		it=Item::new_(production,dot,key);
		key_toItem_[key]=it;
	}else
		it=key_toItem_[key];
	return it;

}
void LALR_Generator::closure(struct ItemSet* itemset)
{
	std::stack<struct Symbol*> unchecked_nodes;
	struct Symbol* uncheck_node;
	std::set<int> added;
	for(std::map<std::string,struct Item*>::iterator it=itemset->items.begin();it!=itemset->items.end();it++)
	{
		if(!it->second->end_with_dot &&!it->second->DotRight()->IsTerminal() && added.count(it->second->DotRight()->id)==0 )
		{
			added.insert(it->second->DotRight()->id);
			unchecked_nodes.push(it->second->DotRight());
		}
	}

	struct Item* newitem;
	while(!unchecked_nodes.empty())
	{
		uncheck_node=unchecked_nodes.top();
		unchecked_nodes.pop();

		//update by this nonterminal
		for(std::vector<struct Production*>::iterator it=uncheck_node->production_list.begin();
			it!=uncheck_node->production_list.end();it++)
		{
			newitem=this->get_or_new(*it,0);
			if(itemset->AddItem(newitem))
			{
				if(!newitem->end_with_dot &&!newitem->DotRight()->IsTerminal() && added.count(newitem->DotRight()->id)==0)
				{
					added.insert(newitem->DotRight()->id);
					unchecked_nodes.push(newitem->DotRight());
				}
			}
		}
	}
}
std::map<struct Symbol*,struct ItemSet*> *LALR_Generator::build_goto_catalogue(struct ItemSet* itemset)
{
	static std::map<struct Symbol*,struct ItemSet*> goto_catalogue;

	struct Item* item;
	goto_catalogue.clear();
	for(std::map<std::string,struct Item*>::iterator it=itemset->items.begin();it!=itemset->items.end();it++)
	{
		item=it->second;
		if(!item->end_with_dot)
		{
			if(goto_catalogue.count(item->DotRight())==0)
			{
				goto_catalogue[item->DotRight()]=ItemSet::new_();
			}
			goto_catalogue[item->DotRight()]->AddKernelItem(this->get_or_new(item->production,item->dot+1));
		}
	}
	return &goto_catalogue;
}
void LALR_Generator::build_canonical_collection()
{
	this->initialItem_=this->get_or_new(this->start_production_,0);
	this->acceptItem_=this->get_or_new(this->start_production_,1);
	this->initial_Set_=ItemSet::new_();
	this->initial_Set_->AddKernelItem(this->initialItem_);
	closure(this->initial_Set_);

	this->initial_Set_->CalculateKey();
	CC_.AddCollection(this->initial_Set_);

	std::stack<struct ItemSet*> unchecked_collection_stack;
	struct ItemSet*   unchecked_collection;
	std::map<struct Symbol*,struct ItemSet*>* goto_catalogue;

	unchecked_collection_stack.push(this->initial_Set_);

	while(!unchecked_collection_stack.empty())
	{
		unchecked_collection=unchecked_collection_stack.top();
		unchecked_collection_stack.pop();
		goto_catalogue=build_goto_catalogue(unchecked_collection);
		for(std::map<struct Symbol*,struct ItemSet*>::iterator it=goto_catalogue->begin();it!=goto_catalogue->end();it++)
		{
			it->second->CalculateKey();
			if(CC_.AddCollection(it->second))
			{
				closure(it->second);
				unchecked_collection_stack.push(it->second);
			}
			unchecked_collection->GOTO[it->first]=CC_.Lookup(it->second->key);
			assert(unchecked_collection->GOTO[it->first]!=NULL);
		}
	}

}

void LALR_Generator::closure_LR(struct ItemSet_LR* itemset_LR)
{
	//only once for each pair <Nonterminal lookahead>.
	std::stack<std::pair<struct Symbol *,struct Symbol *> > unchecked_pairs_stack;
	std::pair<struct Symbol *,struct Symbol *> unchecked_pair;
	std::set<std::string > added;//only once for each pair <Nonterminal lookahead>.
	std::string pair_key;
	std::vector<struct Symbol *>::iterator suffix_iter;
	std::set<int> *suffix_FST;

	struct Symbol *lookahead,*dot_right;
	struct Item* base;

	for(std::map<std::string,struct Item_LR*>::iterator it=itemset_LR->items_LR.begin();
		it!=itemset_LR->items_LR.end();it++)
	{
		lookahead=it->second->lookahead;
		base=it->second->base;
		if(!base->end_with_dot && !base->DotRight()->IsTerminal())
		{
			dot_right=base->DotRight();
			suffix_iter=base->DotRightIter();
			suffix_FST=this->script_->FST_suffix_LR(suffix_iter+1,base->EndOfProductionNodeList(),lookahead);
			for(std::set<int>::iterator it_inner=suffix_FST->begin();it_inner!=suffix_FST->end();it_inner++)
			{
				pair_key=int_int_to_string(dot_right->id,*it_inner);
				if(added.count(pair_key)==0)
				{
					added.insert(pair_key);
					//add unchecked pair
					unchecked_pairs_stack.push(std::make_pair(dot_right,this->script_->LookupId(*it_inner)));
				}
			}
		}
	}
	struct Item_LR* new_item_LR;
	struct Item* new_item;
	while(!unchecked_pairs_stack.empty())
	{
		unchecked_pair=unchecked_pairs_stack.top();
		unchecked_pairs_stack.pop();

		for(std::vector<struct Production*>::iterator it =unchecked_pair.first->production_list.begin();
			it!=unchecked_pair.first->production_list.end();it++)
		{
			new_item=this->get_or_new(*it,0);
			new_item_LR=Item_LR::new_(new_item,unchecked_pair.second);
			if(itemset_LR->AddItem(new_item_LR))
			{
				if(!new_item->end_with_dot && !new_item->DotRight()->IsTerminal())
				{
					dot_right=new_item->DotRight();
					suffix_iter=new_item->DotRightIter();
					suffix_FST=this->script_->FST_suffix_LR(suffix_iter+1,new_item->EndOfProductionNodeList(),unchecked_pair.second);
					for(std::set<int>::iterator it_inner=suffix_FST->begin();it_inner!=suffix_FST->end();it_inner++)
					{
						pair_key=int_int_to_string(dot_right->id,*it_inner);
						if(added.count(pair_key)==0)
						{
							added.insert(pair_key);
							//add unchecked pair
							unchecked_pairs_stack.push(std::make_pair(dot_right,this->script_->LookupId(*it_inner)));
						}
					}
				}
			}
		}
	}
}
void LALR_Generator::build_propagated_and_spontaneous_table_of_item(struct Item* item)
{
	if(item->propagated_and_spontaneous_table_has_build)
		return;
	item->propagated_and_spontaneous_table_has_build=true;

	struct ItemSet_LR* itemset_LR=ItemSet_LR::new_();
	itemset_LR->AddKernelItem(Item_LR::new_(item,this->script_->EndmarkerSymbol()));//treat it as a kernel item,whatever.... do not matter here.
	closure_LR(itemset_LR);

	for(std::map<std::string,struct Item_LR*>::iterator it=itemset_LR->items_LR.begin();
		it!=itemset_LR->items_LR.end();it++)
	{
		if(!it->second->base->end_with_dot)
		{
			if(it->second->lookahead->id!=this->script_->EndmarkerSymbol()->id)
			{//spontaneous
				item->spontaneous_list.push_back(it->second);
			}else
			{//propagated
				item->propagated_list.push_back(it->second->base);
			}
		}
	}
}
void LALR_Generator::discovering_propagated_and_spontaneous_lookaheads_by_item(struct ItemSet* collection,struct Item* item)
{
	build_propagated_and_spontaneous_table_of_item(item);

	struct Item_LR* by_image_item_LR;
	struct Item* from_item;
	struct ItemSet* GOTO_I_X;
	for(std::vector<struct Item_LR*>::iterator it=item->spontaneous_list.begin();it!=item->spontaneous_list.end();it++)
	{//spontaneous_list
		struct Symbol* dot_right=(*it)->base->DotRight();
		GOTO_I_X=collection->GOTO[dot_right];
		assert(GOTO_I_X!=NULL);
		by_image_item_LR=Item_LR::new_(this->get_or_new((*it)->base->production,(*it)->base->dot+1),(*it)->lookahead);
		if(GOTO_I_X->extension_LR->AddKernelItem(by_image_item_LR))
		{
			by_image_item_LR->containing_collection=GOTO_I_X;
			this->unpropagated_stack.push(by_image_item_LR);
		}
	}
	for(std::vector<struct Item*>::iterator it=item->propagated_list.begin();it!=item->propagated_list.end();it++)
	{//propagated_list
		from_item=item;
		GOTO_I_X=collection->GOTO[(*it)->DotRight()];
		assert(GOTO_I_X!=NULL);
		collection->propagated_target_collections_of_item[from_item->key].push_back(GOTO_I_X);
	}
}
void LALR_Generator::discovering_propagated_and_spontaneous_lookaheads()
{
	for(std::map<std::string,struct ItemSet*>::iterator it=CC_.key_to_collection.begin();
		it!=CC_.key_to_collection.end();it++)
	{
		for(std::map<std::string,struct Item*>::iterator it_inner=it->second->kernel_items.begin();
			it_inner!=it->second->kernel_items.end();it_inner++)
		{
			discovering_propagated_and_spontaneous_lookaheads_by_item(it->second,it_inner->second);
		}
	}
	struct Item_LR* initial_item_LR;
	initial_item_LR=Item_LR::new_(this->initialItem_,this->script_->EndmarkerSymbol());
	this->initial_Set_->extension_LR->AddKernelItem(initial_item_LR);
	initial_item_LR->containing_collection=this->initial_Set_;
	this->unpropagated_stack.push(initial_item_LR);
}
void LALR_Generator::do_propagation()
{
	struct Item_LR* unpropagated_item_LR;
	struct Item_LR* image_item_LR;
	struct ItemSet* target;
	std::vector<struct ItemSet*> *propagated_targets;

	std::string key;
	struct ItemSet* containing_collection;
	struct Item* unpropagated_item;
	struct Symbol* lookahead;
	while(!this->unpropagated_stack.empty())
	{
		unpropagated_item_LR=unpropagated_stack.top();
		unpropagated_stack.pop();

		key=unpropagated_item_LR->base->key;
		containing_collection=unpropagated_item_LR->containing_collection;
		propagated_targets=&(containing_collection->propagated_target_collections_of_item[key]);
		for(size_t i=0;i<unpropagated_item_LR->base->propagated_list.size();i++)
		{
			target=propagated_targets->at(i);
			unpropagated_item=unpropagated_item_LR->base->propagated_list[i];
			lookahead=unpropagated_item_LR->lookahead;

			image_item_LR=Item_LR::new_(this->get_or_new(unpropagated_item->production,unpropagated_item->dot+1),lookahead);
			if(target->extension_LR->AddKernelItem(image_item_LR))
			{
				image_item_LR->containing_collection=target;
				this->unpropagated_stack.push(image_item_LR);
			}
		}
	}
}

static void SFHIT_action(struct ItemSet* collection,struct Symbol *dot_right)
{
	if(collection->parsing_table.count(dot_right)!=0)
	{
		if(collection->parsing_table[dot_right].type==Action::REDUCE)
		{
			DEBUG("Conflicting(R-S)On: [ %s ]\n",dot_right->name.c_str());
			DEBUG("EXIST...REDEUCE by %s \n",collection->parsing_table[dot_right].reduce_by_production->toString().c_str());
			DEBUG("NOW.....SHIFT %s \n",dot_right->name.c_str());
			DEBUG("PREFER SHIFT.....\n");
		}
		else
			return;
	}
	collection->parsing_table[dot_right].type=Action::SHIFT;
	collection->parsing_table[dot_right].shift_to_collection=collection->GOTO[dot_right];
}
static void GOTO_action(struct ItemSet* collection,struct Symbol *dot_right)
{
	if(collection->parsing_table.count(dot_right)==0)
	{
		collection->parsing_table[dot_right].type=Action::GOTO;
		collection->parsing_table[dot_right].go_to_collection=collection->GOTO[dot_right];
	}
}
static void REDUCE_action(struct ItemSet* collection,struct Production* production,struct Symbol *lookahead)
{
	if(collection->parsing_table.count(lookahead)!=0)
	{
		if(collection->parsing_table[lookahead].type==Action::REDUCE)
		{
			DEBUG("Conflicting(R-R)On: [ %s ]\n",lookahead->name.c_str());
			DEBUG("EXIST...REDEUCE by %s \n",collection->parsing_table[lookahead].reduce_by_production->toString().c_str());
			DEBUG("NOW.....REDEUCE by %s \n",production->toString().c_str());
			DEBUG("ERROR....\n");
		}else if(collection->parsing_table[lookahead].type==Action::SHIFT)
		{
			DEBUG("Conflicting(S-R)On: [ %s ]\n",lookahead->name.c_str());
			DEBUG("EXIST...SHIFT %s \n",lookahead->name.c_str());
			DEBUG("NOW.....REDEUCE by %s \n",production->toString().c_str());
			DEBUG("PREFER SHIFT.....\n");
		}else
		{
			assert(0);
		}
	}else
	{
		collection->parsing_table[lookahead].type=Action::REDUCE;
		collection->parsing_table[lookahead].reduce_by_production=production;
	}
}
static void ACCEPT_action(struct ItemSet* collection,struct Symbol *lookahead)
{
	DEBUG("\n\nCross ACCEPT Item.\n");
	collection->parsing_table[lookahead].type=Action::ACCEPT;
}
void LALR_Generator::construct_parsing_table_by_collection_item(struct ItemSet* collection,struct Item_LR* item_LR)
{
	struct Item	  *item = item_LR->base;
	struct Symbol *lookahead=item_LR->lookahead;

	if(!item->end_with_dot)
	{
		assert(collection->GOTO[item->DotRight()]!=NULL);
		if(item->DotRight()->IsTerminal())
		{
			SFHIT_action(collection,item->DotRight());
		}
		else
			GOTO_action(collection,item->DotRight());
	}else
	{
		if(item_LR->base->production->id==this->start_production_->id && lookahead->id==this->script_->EndmarkerSymbol()->id)
		{
			ACCEPT_action(collection,lookahead);
		}else
		{
			REDUCE_action(collection,item->production,lookahead);
		}
	}
}
void LALR_Generator::construct_parsing_table_by_collection(struct ItemSet* collection)
{
	for(std::map<std::string,struct Item_LR*>::iterator it=collection->extension_LR->items_LR.begin();
		it!=collection->extension_LR->items_LR.end();it++)
	{
		construct_parsing_table_by_collection_item(collection,it->second);
	}
}
void LALR_Generator::construct_parsing_table()
{
	for(std::vector<struct ItemSet*>::iterator it=CC_.collection_list.begin();
		it!=CC_.collection_list.end();it++)
	{
		closure_LR((*it)->extension_LR);
		construct_parsing_table_by_collection(*it);
	}
}

void LALR_Generator::out_put_parsing_table(std::string to_file)
{
	std::string text;
	{
		for(std::vector<struct Symbol*>::iterator it=this->script_->symbol_list_.begin();
			it!=this->script_->symbol_list_.end();it++)
		{
			text.append(int_to_string((*it)->id));
			text.append(":");
			text.append((*it)->name);
			text.append(";\n");
		}
	}
	text.append("%%\n");
	{
		for(std::vector<struct Production*>::iterator it=this->script_->production_list_.begin();
			it!=this->script_->production_list_.end();it++)
		{
			text.append(int_to_string((*it)->id));
			text.append(":");
			text.append(int_to_string((*it)->length()));
			text.append(":");
			text.append(int_to_string((*it)->head->id));
			text.append(":");
			text.append((*it)->srcipt_action);
			//
			{
				text.append("[");
				for(size_t i=0;i<(*it)->node_list.size();i++)
				{
					struct Symbol* node=(*it)->node_list.at(i);
					if(!node->IsTerminal() && !node->IsEmptySymbol())
					{
						text.append(int_to_string(i));
						text.append(",");
					}
				}
				text.append("]");
			}
			text.append(";\n");
		}
	}
	text.append("%%\n");
	{
		for(std::vector<struct ItemSet*>::iterator it=this->CC_.collection_list.begin();
			it!=this->CC_.collection_list.end();it++)
		{
			text.append(int_to_string((*it)->id));
			text.append(":");
			for(std::map<struct Symbol*,struct Action>::iterator it_inner=(*it)->parsing_table.begin();
				it_inner!=(*it)->parsing_table.end();it_inner++)
			{
				text.append("[");
				text.append(int_to_string(it_inner->first->id));
				text.append(",");
				text.append(int_to_string(it_inner->second.type));
				text.append(",");
				if(it_inner->second.type==Action::SHIFT)
					text.append(int_to_string(it_inner->second.shift_to_collection->id));
				else if(it_inner->second.type==Action::REDUCE)
					text.append(int_to_string(it_inner->second.reduce_by_production->id));
				else if(it_inner->second.type==Action::GOTO)
					text.append(int_to_string(it_inner->second.go_to_collection->id));
				else if(it_inner->second.type==Action::ACCEPT)
					text.append("0");
				text.append("]");
			}
			text.append(";\n");
		}
	}
	text.append("%%\n");
	string_to_file(text,to_file);
}
////////////////////////////////////////////////////////////////
void LALR_Generator::_DEBUG_()
{
	::string_to_file(CC_.toString(),"DEBUG.txt");
}
void LALR_Generator::TEST()
{
	DEBUG("\nLoading Script.......................................\n");
	class Script *sc=new class Script("C_GM.txt");
	class LALR_Generator *LALR_g=new class LALR_Generator();
	LALR_g->script_=sc;
	DEBUG("\ndone!!\n\n");
	DEBUG("\nbuilding canonical collection.........................\n");
	LALR_g->Augment();
	LALR_g->build_canonical_collection();
	DEBUG("\ndone!!\n\n");
	DEBUG("\ndiscovering propagated and spontaneous lookaheads.....\n");
	LALR_g->discovering_propagated_and_spontaneous_lookaheads();
	DEBUG("\ndone!!\n\n");
	DEBUG("\ndo propagation........................................\n");
	LALR_g->do_propagation();
	DEBUG("\ndone!!\n\n");
	DEBUG("\nconstruct parsing table...............................\n");
	LALR_g->construct_parsing_table();;
	DEBUG("\ndone!!\n\n");
	DEBUG("\noutput parsing table to LALR_table.txt......\n");
	LALR_g->out_put_parsing_table("LALR_table.txt");
	DEBUG("\ndone!!\n\n");
	system("pause");
	LALR_g->_DEBUG_();
}



