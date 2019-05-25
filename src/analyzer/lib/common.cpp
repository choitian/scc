#include "common.h"

std::string string_format(const char * format, ...)
{
	static char buff[256];
	std::string text;
	va_list vl;
	va_start(vl,format);

	vsprintf(buff,format,vl);

	va_end(vl);
	text.assign(buff);
	return text;
}
std::string int_to_string(int i)
{
	static char buff[64];
	std::string s;

	sprintf(buff,"%d",i);
	s.append(buff);
	return s;
}
int string_to_int(std::string number)
{
	return atoi(number.c_str());
}
char* file_to_char_array(std::string file,int* size)
{
  FILE * pFile;
  long lSize;
  char * buffer;
  size_t result;

  pFile = fopen (file.c_str() , "rb" );
  if (pFile==NULL) {fputs ("File error",stderr); exit (1);}

  // obtain file size:
  fseek (pFile , 0 , SEEK_END);
  lSize = ftell (pFile);
  if(size!=NULL)
	  *size=lSize;
  rewind (pFile);

  // allocate memory to contain the whole file:
  buffer = new char[lSize+2];
  if (buffer == NULL) {fputs ("Memory error",stderr); exit (2);}

  // copy the file into the buffer:
  result = fread (buffer,1,lSize,pFile);
  if (result != lSize) {fputs ("Reading error",stderr); exit (3);}

  /* the whole file is now loaded in the memory buffer. */
  buffer[lSize]=-1;
  buffer[lSize+1]='\0';
  // terminate
  fclose (pFile);
  return buffer;
}
void string_to_file(std::string string,std::string file)
{
  FILE * pFile;
  size_t result;
  pFile = fopen (file.c_str(), "wb" );
  if (pFile==NULL) {fputs ("File error",stderr); exit (1);}
  result=fwrite (string.c_str(),1 , string.size() , pFile );
  if (result != string.size()) {fputs ("Writing error",stderr); exit (2);}
  fclose (pFile);
}
int skip_white_space(char* src,char* src_end,int single_line_comment,int block_comment,int continue_line,
	int* nskiped,int* nlines)
{
	if(src > src_end)
	{
		return -1;
	}
	int nlines_=0;
	char* start=src;
	char ch=src[0];
	while(ch==' '||ch=='\t' ||ch=='\r'||ch=='\n' ||
		 ch=='\v' ||ch=='\f' ||
		 ch=='/' || ch =='\\')
	{
		if(ch=='/')
		{
			//must least 2 char remain.
			if(src>=src_end)
			{
				break;
			}
			char* src_=src;
			int nlines__=nlines_;
			if(single_line_comment!=0 && src[1]=='/')
			{
				src++;src++;//pass //
				while(1)
				{
					//buffer exhausted
					if(src > src_end)
					{
						break;
					}
					if(src[0]=='\n')
					{
						src++;nlines_++;
						break;
					}else
						src++;
				}
			}else if(block_comment!=0 && src[1]=='*')
			{
				int reset_buffer=0;
				src++;src++;//pass/*
				while(1)
				{
					//buffer exhausted,still no "*/",reset buffer.
					if(src >= src_end)
					{
						reset_buffer=1;
					}
					if(src[0]=='*' && src[1]=='/')
					{
						src++;src++;
						break;
					}else
					{
						if(src[0]=='\n')
							nlines_++;
						src++;
					}
				}
				if(reset_buffer!=0)
				{
					src=src_;
					nlines_=nlines__;
					break;
				}
			}else
			{
				break;
			}
		}else if(ch=='\\')
		{
			if(continue_line!=0)
			{
				if(src+1<=src_end && src[1]=='\n')
				{
					src++;src++;nlines_++;
				}else if(src+1< src_end && src[1]=='\r' && src[2]=='\n')
				{
					src++;src++;src++;nlines_++;
				}else
				{
					break;
				}
			}else 
			{
				break;
			}
		}else if(ch=='\n')
		{
			nlines_++;
			src++;
		}else
		{
			src++;
		}
		if( src > src_end)
		{
			break;
		}
		ch=src[0];
	}
	if(nskiped!=NULL)
		*nskiped=src-start;
	if(nlines!=NULL)
		*nlines=nlines_;
	return 0;
}
int  one_line(char* src,char* src_end,int continue_line,int* size,int* nlines)
{
	int nlines_=0;
	char* start=src;

	if( src > src_end)
	{
		return -1;
	}
	while(1)
	{
		if(src>src_end)
		{
			break;
		}
		if(src[0]=='\n')
		{
			nlines_++;
			src -=2;
			if( continue_line!= 0 &&
				(src[1]=='\\'|| (src[0]=='\\' && src[1]=='\r')))
			{
				src +=3;
			}else
			{
				src +=3;
				break;
			}
		}else
			src++;
	}
	if(size!=NULL)
		*size=src-start;
	if(nlines!=NULL)
		*nlines=nlines_<1?1:nlines_;
	return 0;
}


int SCC_DEBUG(const char * format, ...)
{
  va_list vl;
  va_start(vl,format);

  vfprintf(SCC_ENV.debug_FP,format,vl);

  va_end(vl);
  fprintf(SCC_ENV.debug_FP,"\r\n");
  return 0;
}
int SCC_MSG(const char * format, ...)
{
  va_list vl;
  va_start(vl,format);

  vprintf(format,vl);

  va_end(vl);
  printf("\n");
  return 0;
}

int SCC_ERROR(std::string pos,const char * format, ...)
{
  SCC_ENV.error_num++;
  printf("%s : error : ",pos.c_str());
  va_list vl;
  va_start(vl,format);

  vprintf(format,vl);
  va_end(vl);
  printf("\n");
  return 0;
}
int SCC_SYNTAX_ERROR(std::string pos,const char * format, ...)
{
  SCC_ENV.error_num++;
  printf("%s : syntax error : ",pos.c_str());
  va_list vl;
  va_start(vl,format);

  vprintf(format,vl);
  va_end(vl);
  printf("\n");
  return 0;
}
int SCC_WARNNING(std::string pos,const char * format, ...)
{
  SCC_ENV.warnning_num++;
  printf("%s : warnning :",pos.c_str());
  va_list vl;
  va_start(vl,format);

  vprintf(format,vl);
  va_end(vl);
  printf("\n");
  return 0;
}

size_t ALIGN(size_t size,size_t align)
{
	return (size + align - 1) & (~(align - 1));
}
ENVIRONMENT::ENVIRONMENT()
{
	this->error_num=0;
	this->warnning_num=0;
}