#include <stdio.h>
#include <stddef.h>
typedef struct s
{
	int a;
	unsigned long x;
	int b;
}s_t;
void main()
{
	printf("%d\n",(int)offsetof(s_t,a));
	printf("%d\n",(int)offsetof(s_t,x));
	printf("%d\n",(int)offsetof(s_t,b));
	while(1);
}
