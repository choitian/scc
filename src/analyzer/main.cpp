#define _CRT_SECURE_NO_WARNINGS
#include <Cstdlib> 
#include <Cstdio> 
#include <Cstring>
#include <windows.h>

#include <string> 
#include <vector> 

#include "Syntax_Parser\Syntax_Parser .h"
#include "Semantic\semantic_analyzer.h"

struct ENVIRONMENT SCC_ENV;
void setup_ENVIRONMENT(std::string input_file)
{
	static const char* debug_ext="___DEBUG.txt";
	static const char* ic_ext="___IC.txt";
	static const char* asm_ext=".as";
	SCC_ENV.DFA_table="env\\DFA_state.txt";
	SCC_ENV.LALR_table="env\\LALR_table.txt";
	SCC_ENV.input_file=input_file;


	std::string input_file_name;
	std::string input_file_path;
	size_t len=input_file.size();
	size_t file_name_begin=0;
	size_t file_name_end=len;
	for(size_t i=0;i<len;i++)
	{
		if(input_file[len-i-1] =='.')
		{
			file_name_end=len-i-1;
		}
		if(input_file[len-i-1] =='\\')
		{
			file_name_begin=len-i;
			break;
		}
	}
	input_file_name.assign(input_file.begin()+file_name_begin,input_file.begin()+file_name_end);
	input_file_path.assign(input_file.begin(),input_file.begin()+file_name_begin);

	SCC_ENV.debug_file.assign(input_file_path);
	SCC_ENV.debug_file.append(input_file_name);
	SCC_ENV.debug_file.append(debug_ext);
	SCC_ENV.debug_FP = fopen (SCC_ENV.debug_file.c_str(), "w+" );

	SCC_ENV.ic_file.assign(input_file_path);
	SCC_ENV.ic_file.append(input_file_name);
	SCC_ENV.ic_file.append(ic_ext);

	SCC_ENV.asm_file.assign(input_file_path);
	SCC_ENV.asm_file.append(input_file_name);
	SCC_ENV.asm_file.append(asm_ext);
}
int main(int argc, char *argv[])
{
	/*
		from xx\\scc\\main.cpp  to    xx\\scc\\
		and set currentdirectory to xx\\scc\\
	*/
	if(argc==2)
	{
		setup_ENVIRONMENT(argv[1]);
	}else
	{
		const char* main_file=__FILE__;
		std::string CurrentDirectory;
		CurrentDirectory.assign(main_file,strlen(main_file)-8);
		SCC_MSG("Set CurrentDirectory to %s\n\n",CurrentDirectory.c_str());
		SetCurrentDirectoryA(CurrentDirectory.c_str());

		setup_ENVIRONMENT("test\\test.ccc");
	}
	class Syntax_Parser*  sp=new class Syntax_Parser();
	struct TRANSLATION_UNIT *translation_unit=sp->syntax_parsing();
	struct Semantic_Analyzer* sa=new struct Semantic_Analyzer(translation_unit);
	sa->semantic_analyzing();
	if(SCC_ENV.error_num==0)
		sa->translate();
	SCC_MSG("\nsemantic analyzing done.   Error(%d),Warnning(%d).\n",SCC_ENV.error_num,SCC_ENV.warnning_num);
	return 0;
}

