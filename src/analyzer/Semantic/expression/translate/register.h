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

struct REG
{
	std::string as(unsigned int width);
	void add(struct TEMP* t);
	void set(struct TEMP* t);
	std::string d_name;
	std::string w_name;
	std::string b_name;
	std::vector<struct TEMP*> t_list;
	int spill_cost();
	void spill();
	bool locked;
	void lock();
	void unlock();
	REG(std::string d_name,	std::string w_name,std::string b_name);
};
struct X86_REG
{
	std::list<struct REG*>  reg_list;
	struct REG* get_reg();
	struct REG* get_reg(std::string name);
	X86_REG();
	void spill_all();
};
extern struct X86_REG x86_reg;