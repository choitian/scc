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
#define SAVE_REG_NUMBER 4
#define FIRST_PARAMETER_OFFSET SAVE_REG_NUMBER*4+4
#define PARAMETER_ALIGN 4
#define STACK_ALIGN_SIZE 4


std::string get_access_name(struct IDENTIFIER* identifier);
std::string address_ptr(unsigned int width);
struct MEM* new_floating(struct CONST* value);
