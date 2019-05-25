#define _CRT_SECURE_NO_WARNINGS

#include <Cstdlib> 
#include <Cstdio> 
#include <Cstring>
#include <windows.h>

#include <string> 
#include <vector> 

#include "regular_expression\\regular_expression.h"
#include "LALR_Generator\\LALR_Generator.h"

int main()
{
	/*
		from xx\\scc\\main.cpp  to    xx\\scc\\
		and set currentdirectory to xx\\scc\\
	*/
	const char* main_file=__FILE__;
	std::string CurrentDirectory;
	CurrentDirectory.assign(main_file,strlen(main_file)-8);
	printf("Set CurrentDirectory to %s\n\n",CurrentDirectory.c_str());
	SetCurrentDirectoryA(CurrentDirectory.c_str());

//	regular_expression::TEST();
	LALR_Generator::TEST();
	system("pause");
	return 0;
}

