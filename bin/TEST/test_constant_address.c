int printf ( const char * format, ... );
int ia[]={1,2,3,4,5};
int *afst=&ia[0];
int *asnd=&ia[1];
int *athd=&ia[2];

int itest=12-(12-(int)&ia)+12-2+10+2-10+4;
int main()
{
	printf("%d,%d,%d,%d\n",*afst,*asnd,*athd,*(int*)itest);
	return 0;
}