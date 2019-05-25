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

Semantic_Analyzer::Semantic_Analyzer(struct TRANSLATION_UNIT *node)
{
	this->translation_unit_=node;
}
void Semantic_Analyzer::semantic_analyzing()
{
	if(this->translation_unit_==NULL)
		SCC_MSG("empty translation_unit.\n");
	else
	{
		translation_unit_->check();
	}
}
void Semantic_Analyzer::TEST(struct TRANSLATION_UNIT *node)
{
}