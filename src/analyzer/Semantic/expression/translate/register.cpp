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

#include "..\IC.h"
#include "register.h"

struct X86_REG x86_reg;
REG::REG(std::string d_name,std::string w_name,std::string b_name)
{
	this->d_name=d_name;
	this->w_name=w_name;
	this->b_name=b_name;
	this->locked=false;
}
std::string REG::as(unsigned int width)
{
	if(width==4)
		return this->d_name;
	else if(width==2)
		return this->w_name;
	else if(width==1)
		return this->b_name;
	assert(0);
	return "";
}
void REG::lock()
{
	this->locked=true;
}
void REG::unlock()
{
	this->locked=false;
}
int REG::spill_cost()
{
	int cost=0;
	struct TEMP* t;
	for(std::vector<struct TEMP*>::iterator it=t_list.begin();it!=t_list.end();it++)
	{
		t=*it;
		if(t->ref>0)
		{
			cost++;
		}
	}
	return cost;
}
void REG::spill()
{
	for(std::vector<struct TEMP*>::iterator it=t_list.begin();it!=t_list.end();it++)
	{
		struct TEMP* t=*it;
		if(t->ref>0)
		{
			t->save(this);
		}
		t->reg=NULL;
	}
	t_list.clear();
}
void REG::add(struct TEMP* t)
{
	t_list.push_back(t);
	t->reg=this;
}
void REG::set(struct TEMP* t)
{
	t_list.clear();
	t_list.push_back(t);
	t->reg=this;
}


X86_REG::X86_REG()
{
	this->reg_list.push_back(new REG("ebx","bx","bl"));
	this->reg_list.push_back(new REG("ecx","cx","cl"));
	this->reg_list.push_back(new REG("edx","dx","dl"));
	this->reg_list.push_back(new REG("eax","ax","al"));
	this->reg_list.push_back(new REG("esi","si",""));
	this->reg_list.push_back(new REG("edi","di",""));
}
struct REG* X86_REG::get_reg()
{
	int min_cost=1024;
	struct REG* min_cost_r=NULL;
	for(std::list<struct REG*>::iterator it=this->reg_list.begin();it!=reg_list.end();it++)
	{
		struct REG* r=*it;
		if(r->locked)
			continue;
		int r_cost=r->spill_cost();
		if(r_cost<min_cost)
		{
			min_cost=r_cost;
			min_cost_r=r;
			if(min_cost_r<=0)
				break;
		}
	}
	assert(min_cost_r!=NULL);
	min_cost_r->spill();
	return min_cost_r;
}
struct REG* X86_REG::get_reg(std::string name)
{
	struct REG* r = NULL;
	bool find=false;
	for(std::list<struct REG*>::iterator it=this->reg_list.begin();it!=reg_list.end();it++)
	{
		r=*it;
		if(r->d_name==name)
		{
			find=true;
			break;
		}
	}
	assert(find);
	r->spill();
	return r;
}
void X86_REG::spill_all()
{
	for(std::list<struct REG*>::iterator it=this->reg_list.begin();it!=reg_list.end();it++)
	{
		struct REG* r=*it;
		r->spill();
	}
}
