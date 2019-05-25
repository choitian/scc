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

struct Semantic_Analyzer
{
	static void TEST(struct TRANSLATION_UNIT *node);
	Semantic_Analyzer(struct TRANSLATION_UNIT *node);
	void semantic_analyzing();
	void translate();
private:
	struct TRANSLATION_UNIT *translation_unit_;
};