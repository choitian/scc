#define _CRT_SECURE_NO_WARNINGS
#include "common.h"



std::string int_to_string(int i)
{
	static char buff[64];
	std::string s;

	sprintf(buff,"%d",i);
	s.append(buff);
	return s;
}
std::string int_int_to_string(int i,int j)
{
	static char buff[64];
	std::string s;

	sprintf(buff,"%d-%d",i,j);
	s.append(buff);
	return s;
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

int DEBUG(const char * format, ...)
{
  va_list vl;
  va_start(vl,format);

  vprintf(format,vl);

  va_end(vl);
  return 0;
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