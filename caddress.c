#include <stdio.h>
struct s
{
	int a;
};
int add(int a, int b);
struct s adds(int a, int b);
int main()
{
	int x=100,y=200;
	int sum;
	int (*p)(int,int);
	p = add;
	printf("main:%p\n",p);
	sum=add(x,y);
	printf("adds()=%p\n",&adds(x,y));
	return sum;
}
int add(int x,int y)
{
	int s;
	printf("s=%p\n",&s);
	int (*p)(int,int);
	p = add;
	printf("ad:%p\n",p);
	s=x+y;
	return s;
}
struct s add(int x,int y)
{
	struct s;
	return s;
}
